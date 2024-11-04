import glob
import subprocess
import sys


def run_test(test: str, path_to_built_app: str) -> str:
    result = subprocess.run(
        [path_to_built_app, test.split()[0], test.split()[1]],
        input=test.encode("utf-8"),
        capture_output=True,
    )

    if not result.stdout:
        return result.stderr.decode("utf-8")

    return result.stdout.decode("utf-8")


def check_number_of_arguments(argc: int) -> bool:
    if argc != 2:
        print("Usage: {path_to_test_runner} {path_to_built_app}")
        return False
    return True


def run_tests() -> None:
    if (not check_number_of_arguments(len(sys.argv))):
        return

    path_to_built_app = sys.argv[1]

    test_input_files = sorted(glob.glob("test/test*_input.txt"))
    test_output_files = sorted(glob.glob("test/test*_output.txt"))

    if len(test_input_files) != len(test_output_files):
        print("Error: different amount of input and output files")
        return

    for i, (test_input_file, test_output_file) in enumerate(
        zip(test_input_files, test_output_files), start=1
    ):
        print(f"Running Test #{i}...")
        with open(test_input_file, 'r', encoding='utf-8') as f:
            test_input = f.read().strip()

        with open(test_output_file, 'r', encoding='utf-8') as f:
            expected = f.read().strip()

        actual = run_test(test_input, path_to_built_app)
        actual = actual.replace("\r\n", "\n").replace("\"", "")
        if expected.strip() != actual.strip():
            print(f"\033[31mError\033[0m: Test #{i} failed")
            print(test_input)
            print("Expected:")
            print(expected)
            print("Actual:")
            print(actual)
            break
        print(f"Test #{i} - \033[32mOK\033[0m")

    print("Done")


run_tests()
