# Test doubles

How tests stand in for the interfaces the code under test uses.

A double is a trompeloeil mock, kept in the `mock/` directory beside the
tests of its area, or in the test file when only that file uses it. It states
the calls it expects where they happen, and a check on an argument that only
lives during the call goes in the expectation's `WITH`. A double does not copy
what it saw into a log or a counter for the test to read back afterwards.

trompeloeil cannot mock `operator()` or a template, so a callable the code
under test takes by value is a small copyable forwarder onto a mock with
concrete signatures. A mocked call holds trompeloeil's global lock, so it must
never wait for another mocked call.

Hand written types remain where they are not doubles of an interface: types a
template under test is instantiated with or derived from, test inputs, and a
double the code under test constructs itself before a test can reach it.
