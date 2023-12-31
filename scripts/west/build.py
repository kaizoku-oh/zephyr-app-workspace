import subprocess
import re
import sys

def run_build_command(build_command):
    try:
        # Run the provided build command
        process = subprocess.Popen(build_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        output, error = process.communicate()

        # Check for errors
        if process.returncode != 0:
            print(f"Error occurred: {error.decode('utf-8')}")
            return None

        return output.decode('utf-8')

    except Exception as e:
        print(f"An error occurred: {str(e)}")
        return None

def extract_build_footprint(output):
    # Define the regular expression pattern to extract memory regions and sizes
    pattern = r"Memory region\s+Used Size\s+Region Size\s+%age Used\n(.+?)\n\n"

    # Find the memory region information using regex
    match = re.search(pattern, output, re.DOTALL)
    if match:
        return match.group(1)
    else:
        return None

if __name__ == "__main__":
    # Check if a build command is provided as an argument
    if len(sys.argv) < 2:
        print("Please provide the 'west build' command as an argument.")
        sys.exit(1)

    # Get the 'west build' command from the command line arguments
    build_command = sys.argv[1]

    # Run the 'west build' command and capture its output
    build_output = run_build_command(build_command)

    if build_output:
        # Extract and print the build footprint
        build_footprint = extract_build_footprint(build_output)
        if build_footprint:
            print("Build Footprint:")
            print(build_footprint)
        else:
            print("Unable to extract build footprint.")
