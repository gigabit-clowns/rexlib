import argparse
import toml

def __process(input_path: str, output_path: str, suffix: str):
    with open(input_path, "r") as f:
        data = toml.load(f)
    
    data["project"]["name"] = f"{data['project']['name']}_{suffix}"
    
    with open(output_path, "w") as f:
        toml.dump(data, f)
        
if __name__ == "__main__":
    argparser = argparse.ArgumentParser('Patch pyproject.toml with CUDA suffix')
    argparser.add_argument(
        '-i',
        '--input',
        required=True,
        help='Path to the pyproject.toml file'
    )
    argparser.add_argument(
        '-o',
        '--output',
        help='Path to the output pyproject.toml file. May be empty in '
        'which case the input file will be modified in place'
    )
    argparser.add_argument(
        '--suffix', 
        required=True, 
        help='CUDA version suffix to append to the package name'
    )
    args = argparser.parse_args()
    
    input_path = args.input
    output_path = args.output if args.output else input_path
    suffix = args.suffix
    __process(input_path, output_path, suffix)
