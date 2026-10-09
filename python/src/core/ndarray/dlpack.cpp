// SPDX-License-Identifier: GPL-3.0-only

#include "dlpack.hpp"

#include "dlpack_buffer.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/host_access.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <dlpack/dlpack.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

namespace
{

const char* get_capsule_name(const DLManagedTensor* /*tag*/) noexcept
{
	return "dltensor";
}

const char* get_capsule_name(const DLManagedTensorVersioned* /*tag*/) noexcept
{
	return "dltensor_versioned";
}

const char* get_used_capsule_name(const DLManagedTensor* /*tag*/) noexcept
{
	return "used_dltensor";
}

const char*
get_used_capsule_name(const DLManagedTensorVersioned* /*tag*/) noexcept
{
	return "used_dltensor_versioned";
}

template <typename Managed>
bool holds_tensor(PyObject *capsule) noexcept
{
	const auto *name = get_capsule_name(static_cast<const Managed*>(nullptr));
	return PyCapsule_IsValid(capsule, name) != 0;
}

template <typename Managed>
Managed* get_tensor(PyObject *capsule) noexcept
{
	const auto *name = get_capsule_name(static_cast<const Managed*>(nullptr));
	return static_cast<Managed*>(PyCapsule_GetPointer(capsule, name));
}

void set_version_and_flags(
	DLManagedTensor& /*managed*/,
	std::uint64_t /*flags*/
) noexcept
{
	// A tensor without a version has no field for either.
}

void set_version_and_flags(
	DLManagedTensorVersioned &managed,
	std::uint64_t flags
) noexcept
{
	managed.version.major = DLPACK_MAJOR_VERSION;
	managed.version.minor = DLPACK_MINOR_VERSION;
	managed.flags = flags;
}

void check_version(const DLManagedTensor& /*managed*/) noexcept
{
	// A tensor without a version has one layout only.
}

void check_version(const DLManagedTensorVersioned &managed)
{
	// A tensor of another major version is laid out in another way.
	if (managed.version.major != DLPACK_MAJOR_VERSION)
	{
		std::ostringstream oss;
		oss << "The tensor follows version " << managed.version.major
			<< " of DLPack, and only version " << DLPACK_MAJOR_VERSION
			<< " is understood.";
		throw py::buffer_error(oss.str());
	}
}

bool
get_dlpack_type_code(numerical_type type, std::uint8_t &code) noexcept
{
	switch (get_category(type))
	{
	case numerical_type_category::boolean:
		code = kDLBool;
		return true;
	case numerical_type_category::signed_integer:
		code = kDLInt;
		return true;
	case numerical_type_category::unsigned_integer:
		code = kDLUInt;
		return true;
	case numerical_type_category::floating_point:
		code = kDLFloat;
		return true;
	case numerical_type_category::complex:
		code = kDLComplex;
		return true;
	default:
		return false;
	}
}

DLDataType get_dlpack_data_type(numerical_type type)
{
	DLDataType result;
	if (!get_dlpack_type_code(type, result.code))
	{
		std::ostringstream oss;
		oss << "DLPack has no data type for " << type << ".";
		throw py::buffer_error(oss.str());
	}

	result.bits = static_cast<std::uint8_t>(8 * get_size(type));
	result.lanes = 1;
	return result;
}

bool is_dlpack_data_type(numerical_type type, const DLDataType &other) noexcept
{
	std::uint8_t code = 0;
	return
		get_dlpack_type_code(type, code) &&
		code == other.code &&
		8 * get_size(type) == other.bits &&
		other.lanes == 1;
}

numerical_type get_numerical_type(const DLDataType &type)
{
	const auto count = static_cast<int>(numerical_type::count);
	for (int i = 0; i < count; ++i)
	{
		const auto candidate = static_cast<numerical_type>(i);
		if (is_dlpack_data_type(candidate, type))
		{
			return candidate;
		}
	}

	std::ostringstream oss;
	oss << "No array holds the data type of the tensor (code "
		<< static_cast<int>(type.code) << ", " << static_cast<int>(type.bits)
		<< " bits, " << type.lanes << " lanes).";
	throw py::buffer_error(oss.str());
}

/**
 * A tensor handed out through DLPack, with what it points to: the storage
 * it shares and the shape and strides that describe it.
 */
template <typename Managed>
class dlpack_export
{
public:
	dlpack_export(array &source, void *data, std::uint64_t flags);
	dlpack_export(const dlpack_export &other) = delete;
	dlpack_export(dlpack_export &&other) = delete;
	~dlpack_export() = default;

	dlpack_export& operator=(const dlpack_export &other) = delete;
	dlpack_export& operator=(dlpack_export &&other) = delete;

	Managed* get_managed() noexcept;

private:
	std::shared_ptr<buffer> m_storage;
	std::vector<std::int64_t> m_shape;
	std::vector<std::int64_t> m_strides;
	Managed m_managed;

	static void destroy(Managed *managed);
};

template <typename Managed>
dlpack_export<Managed>::dlpack_export(
	array &source,
	void *data,
	std::uint64_t flags
)
	: m_storage(source.share_storage())
	, m_managed()
{
	const auto &descriptor = source.get_descriptor();
	const auto &layout = descriptor.get_layout();
	const auto data_type = descriptor.get_data_type();

	std::vector<std::size_t> extents;
	layout.get_extents(extents);
	m_shape.assign(extents.cbegin(), extents.cend());

	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);
	m_strides.assign(strides.cbegin(), strides.cend());

	const auto first_byte =
		layout.get_offset() * static_cast<std::ptrdiff_t>(get_size(data_type));

	auto &tensor = m_managed.dl_tensor;
	tensor.data = static_cast<char*>(data) + first_byte;
	tensor.device.device_type = kDLCPU;
	tensor.device.device_id = 0;
	tensor.ndim = static_cast<std::int32_t>(m_shape.size());
	tensor.dtype = get_dlpack_data_type(data_type);
	tensor.shape = m_shape.data();
	tensor.strides = m_strides.data();
	tensor.byte_offset = 0;

	set_version_and_flags(m_managed, flags);
	m_managed.manager_ctx = this;
	m_managed.deleter = &destroy;
}

template <typename Managed>
Managed* dlpack_export<Managed>::get_managed() noexcept
{
	return &m_managed;
}

template <typename Managed>
void dlpack_export<Managed>::destroy(Managed *managed)
{
	delete static_cast<dlpack_export*>(managed->manager_ctx);
}

template <typename Managed>
void destroy_capsule(PyObject *capsule)
{
	// A consumer renames the capsule when it takes the tensor over.
	if (holds_tensor<Managed>(capsule))
	{
		auto *managed = get_tensor<Managed>(capsule);
		managed->deleter(managed);
	}
}

template <typename Managed>
py::capsule make_capsule(array &source, void *data, std::uint64_t flags)
{
	auto exported =
		std::make_unique<dlpack_export<Managed>>(source, data, flags);

	py::capsule result(
		exported->get_managed(),
		get_capsule_name(static_cast<const Managed*>(nullptr)),
		&destroy_capsule<Managed>
	);

	// The capsule owns the export from here on.
	(void)exported.release();
	return result;
}

/**
 * Refuses an array whose memory can not be handed out through DLPack: one
 * that is not initialized, or whose storage the host can not reach.
 */
void check_exportable(const array &source)
{
	const auto *storage = source.get_storage();
	if (storage == nullptr)
	{
		throw py::buffer_error("The array is not initialized.");
	}

	if (!is_host_accessible(storage->get_memory_resource().get_kind()))
	{
		throw py::buffer_error(
			"The storage of the array can not be reached from the host, and "
			"only host memory is exchanged through DLPack."
		);
	}
}

py::capsule to_dlpack(array &source, bool versioned, bool copied)
{
	check_exportable(source);

	void *data = nullptr;
	{
		const py::gil_scoped_release release;
		data = get_host_data(array_ref(source));
	}

	if (versioned)
	{
		return make_capsule<DLManagedTensorVersioned>(
			source,
			data,
			copied ? DLPACK_FLAG_BITMASK_IS_COPIED : 0
		);
	}

	return make_capsule<DLManagedTensor>(source, data, 0);
}

py::tuple get_dlpack_device(const array &source)
{
	check_exportable(source);
	return py::make_tuple(static_cast<int>(kDLCPU), 0);
}

py::tuple get_dlpack_version()
{
	return py::make_tuple(DLPACK_MAJOR_VERSION, DLPACK_MINOR_VERSION);
}

void check_device(const DLTensor &tensor)
{
	if (tensor.device.device_type != kDLCPU)
	{
		throw py::buffer_error(
			"The tensor is not in host memory, and only host memory is "
			"exchanged through DLPack."
		);
	}
}

/**
 * How many elements lie between the lowest address a layout reaches and its
 * first element. The two differ when a stride is negative.
 */
std::ptrdiff_t compute_offset(
	const std::vector<std::size_t> &extents,
	const std::vector<std::ptrdiff_t> &strides
) noexcept
{
	std::ptrdiff_t result = 0;

	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		if (extents[i] == 0)
		{
			return 0;
		}

		if (strides[i] < 0)
		{
			const auto last_index = static_cast<std::ptrdiff_t>(extents[i] - 1);
			result -= last_index * strides[i];
		}
	}

	return result;
}

strided_layout make_layout(const DLTensor &tensor)
{
	const auto rank = static_cast<std::size_t>(tensor.ndim);
	const std::vector<std::size_t> extents(tensor.shape, tensor.shape + rank);

	// A tensor without strides is contiguous, with its last axis fastest.
	if (tensor.strides == nullptr)
	{
		return strided_layout::make_contiguous_layout(make_span(extents));
	}

	const std::vector<std::ptrdiff_t> strides(
		tensor.strides,
		tensor.strides + rank
	);

	return strided_layout::make_custom_layout(
		make_span(extents),
		make_span(strides),
		compute_offset(extents, strides)
	);
}

void check_alignment(const void *data, numerical_type type)
{
	const auto alignment = get_size(make_real(type));
	if (reinterpret_cast<std::uintptr_t>(data) % alignment != 0)
	{
		std::ostringstream oss;
		oss << "The memory of the tensor is not aligned for " << type << ".";
		throw py::buffer_error(oss.str());
	}
}

template <typename Managed>
array import_tensor(PyObject *capsule)
{
	auto *managed = get_tensor<Managed>(capsule);
	check_version(*managed);

	const auto &tensor = managed->dl_tensor;
	check_device(tensor);

	const auto data_type = get_numerical_type(tensor.dtype);
	const auto item_size = get_size(data_type);
	auto layout = make_layout(tensor);

	auto *first = static_cast<char*>(tensor.data) + tensor.byte_offset;
	check_alignment(first, data_type);

	auto storage = std::make_shared<dlpack_buffer>(
		managed,
		first - layout.get_offset() * static_cast<std::ptrdiff_t>(item_size),
		layout.compute_storage_requirement() * item_size
	);

	// The buffer owns the tensor from here on, and the capsule must not
	// release it again.
	const auto *used_name =
		get_used_capsule_name(static_cast<const Managed*>(nullptr));
	if (PyCapsule_SetName(capsule, used_name) != 0)
	{
		throw py::error_already_set();
	}

	return array(
		std::move(storage),
		array_descriptor(std::move(layout), data_type)
	);
}

array from_dlpack_capsule(const py::capsule &capsule)
{
	if (holds_tensor<DLManagedTensorVersioned>(capsule.ptr()))
	{
		return import_tensor<DLManagedTensorVersioned>(capsule.ptr());
	}

	if (holds_tensor<DLManagedTensor>(capsule.ptr()))
	{
		return import_tensor<DLManagedTensor>(capsule.ptr());
	}

	throw py::value_error(
		"The capsule holds no DLPack tensor. A tensor can be taken out of "
		"its capsule only once."
	);
}

bool is_dlpack_capsule_read_only(const py::capsule &capsule)
{
	// A tensor without a version has no way to say it is read only.
	if (!holds_tensor<DLManagedTensorVersioned>(capsule.ptr()))
	{
		return false;
	}

	const auto *managed =
		get_tensor<DLManagedTensorVersioned>(capsule.ptr());
	check_version(*managed);
	return (managed->flags & DLPACK_FLAG_BITMASK_READ_ONLY) != 0;
}

} // anonymous namespace

void bind_dlpack(pybind11::module_ &m)
{
	m.def(
		"to_dlpack",
		&to_dlpack,
		py::arg("array"), py::arg("versioned"), py::arg("copied")
	);
	m.def(
		"get_dlpack_device",
		&get_dlpack_device,
		py::arg("array")
	);
	m.def("get_dlpack_version", &get_dlpack_version);
	m.def(
		"from_dlpack_capsule",
		&from_dlpack_capsule,
		py::arg("capsule")
	);
	m.def(
		"is_dlpack_capsule_read_only",
		&is_dlpack_capsule_read_only,
		py::arg("capsule")
	);
}

} // namespace rexlib
