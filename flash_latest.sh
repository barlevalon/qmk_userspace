#!/bin/bash
# QMK Firmware Flasher for Split Keyboards
# This script downloads and flashes the latest firmware from GitHub
# Actions workflows to both halves of a split keyboard.
set -o pipefail

#################################################
# USER CONFIGURATION - CUSTOMIZE THESE VALUES
#################################################

# GitHub repository containing your QMK userspace
REPO="barlevalon/qmk_userspace"

# Different keyboards use different bootloaders with different mount points
# This will be set based on the keyboard type selected

# GitHub workflow artifact name
FIRMWARE_NAME="Firmware"

# Keyboard selection
KEYBOARD=${1:-"charybdis"} # Default to Charybdis if no argument provided

# Set firmware file and flashing method based on selected keyboard
case "$KEYBOARD" in
  "charybdis"|"ch")
    FIRMWARE_FILE="bastardkb_charybdis_3x6__hearter.uf2"
    FIRMWARE_EXTENSION=".uf2"
    FLASH_METHOD="copy"
    MOUNT_POINT="/Volumes/RPI-RP2"
    ;;
  "corne"|"crkbd"|"co")
    FIRMWARE_FILE="crkbd_rev1_hearter.hex"
    FIRMWARE_EXTENSION=".hex"
    FLASH_METHOD="avrdude"
    ;;
  *)
    echo "Unknown keyboard: $KEYBOARD"
    echo "Available options: charybdis (ch), corne (co/crkbd)"
    exit 1
    ;;
esac

# 1Password item for GitHub token (optional)
OP_GITHUB_TOKEN_PATH="op://Private/github/token"

# Timeout for keyboard reboot (seconds)
KEYBOARD_REBOOT_TIMEOUT=15

# Delay between halves (seconds)
DELAY_BETWEEN_HALVES=2

# The firmware extension will be set by the keyboard selection

# Minimum expected firmware file size (bytes)
MIN_FIRMWARE_SIZE=10000

#################################################
# APPEARANCE SETTINGS
#################################################

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored messages
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check if GitHub CLI is installed
check_gh_cli() {
    if ! command -v gh &> /dev/null; then
        print_message "$RED" "Error: GitHub CLI (gh) is not installed."
        print_message "$YELLOW" "Please install it with: brew install gh"
        exit 1
    fi
}

# Function to check if 1Password CLI is installed
check_op_cli() {
    if ! command -v op &> /dev/null; then
        print_message "$RED" "Error: 1Password CLI is not installed."
        print_message "$YELLOW" "Please install it with: brew install --cask 1password/tap/1password-cli"
        exit 1
    fi
}

# Function to authenticate with GitHub
authenticate() {
    print_message "$BLUE" "Authenticating with GitHub..."
    
    if [[ -z "${GH_TOKEN}" ]]; then
        # Try to get the token from 1Password
        if ! GH_TOKEN=$(op read "$OP_GITHUB_TOKEN_PATH" 2>/dev/null); then
            print_message "$RED" "Failed to get GitHub token from 1Password."
            print_message "$YELLOW" "Trying to authenticate with gh auth status..."
            
            if ! gh auth status &>/dev/null; then
                print_message "$RED" "Not authenticated with GitHub. Please run 'gh auth login' or provide a token."
                exit 1
            else
                print_message "$GREEN" "Already authenticated with GitHub."
            fi
        else
            export GH_TOKEN
            print_message "$GREEN" "Successfully retrieved GitHub token from 1Password."
        fi
    fi
}

# Function to get the latest workflow run status
get_workflow_status() {
    local status
    status=$(gh run list --repo "$REPO" --limit 1 --json status -q '.[0].status' 2>/dev/null) || {
        print_message "$RED" "Failed to get workflow status."
        exit 1
    }
    echo "$status"
}

# Function to get the latest workflow run conclusion
get_workflow_conclusion() {
    local conclusion
    conclusion=$(gh run list --repo "$REPO" --limit 1 --json conclusion -q '.[0].conclusion' 2>/dev/null) || {
        print_message "$RED" "Failed to get workflow conclusion."
        exit 1
    }
    echo "$conclusion"
}

# Function to validate firmware file
validate_firmware() {
    local firmware_path=$1
    local filename=$(basename "$firmware_path")
    local filesize=$(stat -f%z "$firmware_path" 2>/dev/null || stat -c%s "$firmware_path" 2>/dev/null)
    local extension="${filename##*.}"
    
    # Check if file exists
    if [[ ! -f "$firmware_path" ]]; then
        print_message "$RED" "ERROR: Firmware file does not exist at path: $firmware_path"
        return 1
    fi
    
    # Check file extension
    if [[ ! "$filename" =~ $FIRMWARE_EXTENSION$ ]]; then
        print_message "$RED" "ERROR: Firmware file has wrong extension: .$extension (expected $FIRMWARE_EXTENSION)"
        print_message "$YELLOW" "This might not be a valid firmware file."
        return 1
    fi
    
    # Check minimum file size
    if (( filesize < MIN_FIRMWARE_SIZE )); then
        print_message "$RED" "ERROR: Firmware file is too small: $filesize bytes"
        print_message "$YELLOW" "This might not be a valid firmware file or it might be corrupted."
        return 1
    fi
    
    print_message "$GREEN" "Firmware validation passed: $filename ($filesize bytes)"
    return 0
}

# Function to get firmware
get_firmware() {
    local run_id=$1
    print_message "$BLUE" "Downloading firmware artifacts..."
    
    # Create a temporary directory for the firmware
    local tmp_dir
    tmp_dir=$(mktemp -d)
    
    # Download the firmware
    if [[ -n "$run_id" ]]; then
        # Download from specific run ID
        if ! gh run download --repo "$REPO" --name "$FIRMWARE_NAME" --dir "$tmp_dir" "$run_id"; then
            print_message "$RED" "ERROR: Failed to download firmware artifacts from run $run_id."
            print_message "$YELLOW" "Check if the run ID exists and has completed successfully."
            rm -rf "$tmp_dir"
            exit 1
        fi
    else
        # Download from latest run
        if ! gh run download --repo "$REPO" --name "$FIRMWARE_NAME" --dir "$tmp_dir"; then
            print_message "$RED" "ERROR: Failed to download firmware artifacts from latest run."
            print_message "$YELLOW" "Ensure your workflow has completed successfully and includes the '$FIRMWARE_NAME' artifact."
            rm -rf "$tmp_dir"
            exit 1
        fi
    fi
    
    # Look for the firmware file
    local firmware_path
    firmware_path=$(find "$tmp_dir" -name "$FIRMWARE_FILE" | head -1)
    
    # If not found, try to find any file with the right extension
    if [[ -z "$firmware_path" ]]; then
        firmware_path=$(find "$tmp_dir" -type f -name "*${FIRMWARE_EXTENSION}" | head -1)
        if [[ -n "$firmware_path" ]]; then
            print_message "$YELLOW" "Found alternative firmware file: $(basename "$firmware_path")"
            print_message "$YELLOW" "This is not the expected filename ($FIRMWARE_FILE). Proceed with caution."
        else
            print_message "$RED" "ERROR: No firmware files found in the downloaded artifacts."
            print_message "$BLUE" "Contents of download:"
            find "$tmp_dir" -type f | while read -r file; do
                echo "  - $(basename "$file")"
            done
            print_message "$YELLOW" "Check your workflow configuration to ensure it's producing the correct firmware file."
            rm -rf "$tmp_dir"
            exit 1
        fi
    else
        print_message "$GREEN" "Found firmware file: $(basename "$firmware_path")"
    fi
    
    # Validate the firmware file
    if ! validate_firmware "$firmware_path"; then
        print_message "$YELLOW" "Proceeding with caution due to firmware validation issues..."
    fi
    
    FIRMWARE_PATH="$firmware_path"
    TMP_DIR="$tmp_dir"
}

# Function to wait for the keyboard to be connected
wait_for_keyboard() {
    # Only used for RP2040-based keyboards using the copy method
    # For AVR-based keyboards, avrdude handles this
    
    local side=$1
    local attempt=0
    local max_attempts=180  # 3 minutes timeout
    
    print_message "$YELLOW" "Please connect the $side half of your keyboard now."
    print_message "$YELLOW" "Put it in bootloader mode by double-tapping the reset button."
    print_message "$BLUE" "Waiting for device to appear at $MOUNT_POINT..."
    
    local keep_trying=true
    
    while $keep_trying; do
        if [[ -d "$MOUNT_POINT" ]]; then
            print_message "$GREEN" "$side half of the keyboard detected at $MOUNT_POINT"
            
            # Additional validation that this is actually a bootloader device
            if [[ -f "$MOUNT_POINT/INFO_UF2.TXT" ]]; then
                print_message "$GREEN" "Bootloader confirmed - ready to flash!"
                return 0
            else
                # Ask user what to do
                print_message "$YELLOW" "Found $MOUNT_POINT but it doesn't appear to be in bootloader mode."
                print_message "$BLUE" "Options:"
                print_message "$BLUE" "  [r] - Retry detection"
                print_message "$BLUE" "  [p] - Proceed anyway (might not work)"
                print_message "$BLUE" "  [s] - Skip flashing this half (not recommended)"
                print_message "$BLUE" "  [q] - Quit"
                read -p "What would you like to do? [r/p/s/q]: " choice
                
                case "$choice" in
                    r|R) 
                        print_message "$BLUE" "Retrying..."
                        # Just loop again
                        ;;
                    p|P)
                        print_message "$YELLOW" "Proceeding without bootloader confirmation. This might not work."
                        return 0
                        ;;
                    s|S)
                        print_message "$YELLOW" "Skipping $side half. Your keyboard may not work correctly."
                        return 0
                        ;;
                    q|Q)
                        print_message "$RED" "Quitting at user request."
                        exit 1
                        ;;
                    *)
                        print_message "$YELLOW" "Invalid choice. Retrying..."
                        ;;
                esac
                
                # Reset attempt counter when user interacts
                attempt=0
            fi
        else
            # Show a message every 10 seconds or allow user to interact
            if (( attempt % 10 == 0 && attempt > 0 )); then
                print_message "$BLUE" "Still waiting for keyboard... (${attempt}s)"
                print_message "$YELLOW" "Remember to put it in bootloader mode by double-tapping the reset button."
                print_message "$BLUE" "Options:"
                print_message "$BLUE" "  [c] - Continue waiting"
                print_message "$BLUE" "  [s] - Skip flashing this half (not recommended)"
                print_message "$BLUE" "  [q] - Quit"
                read -t 5 -p "What would you like to do? [c/s/q]: " choice || { choice="c"; echo ""; }
                
                case "$choice" in
                    c|C|"") 
                        print_message "$BLUE" "Continuing to wait..."
                        ;;
                    s|S)
                        print_message "$YELLOW" "Skipping $side half. Your keyboard may not work correctly."
                        return 0
                        ;;
                    q|Q)
                        print_message "$RED" "Quitting at user request."
                        exit 1
                        ;;
                    *)
                        print_message "$YELLOW" "Invalid choice. Continuing to wait..."
                        ;;
                esac
            fi
            
            sleep 1
            (( attempt++ ))
            
            # Check for timeout
            if (( attempt >= max_attempts )); then
                print_message "$RED" "Exceeded maximum wait time (${max_attempts}s)."
                print_message "$BLUE" "Options:"
                print_message "$BLUE" "  [r] - Retry detection"
                print_message "$BLUE" "  [s] - Skip flashing this half (not recommended)"
                print_message "$BLUE" "  [q] - Quit"
                read -p "What would you like to do? [r/s/q]: " choice
                
                case "$choice" in
                    r|R) 
                        print_message "$BLUE" "Retrying..."
                        attempt=0
                        ;;
                    s|S)
                        print_message "$YELLOW" "Skipping $side half. Your keyboard may not work correctly."
                        return 0
                        ;;
                    q|Q)
                        print_message "$RED" "Quitting at user request."
                        exit 1
                        ;;
                    *)
                        print_message "$YELLOW" "Invalid choice. Retrying..."
                        attempt=0
                        ;;
                esac
            fi
        fi
    done
}

# Function to check if avrdude is installed
check_avrdude() {
    if ! command -v avrdude &> /dev/null; then
        print_message "$RED" "Error: avrdude is not installed."
        print_message "$YELLOW" "Please install it with: brew install avrdude"
        return 1
    fi
    return 0
}

# Function to flash the firmware to the keyboard
flash_firmware() {
    local file=$1
    local side=$2
    
    if [[ ! -f "$file" ]]; then
        print_message "$RED" "Firmware file not found: $file"
        return 1
    fi
    
    print_message "$BLUE" "Flashing $side half with $(basename "$file")"
    
    # Different flashing method based on the keyboard type
    if [[ "$FLASH_METHOD" == "copy" ]]; then
        # RP2040-based boards like Charybdis use UF2 format and copy method
        print_message "$YELLOW" "Please put the keyboard in bootloader mode (double press reset button)"
        
        # Wait for the keyboard to appear
        wait_for_keyboard "$side"
        
        # Copy the firmware file to the keyboard
        if ! cp "$file" "$MOUNT_POINT/"; then
            print_message "$RED" "Failed to copy firmware to $MOUNT_POINT"
            return 1
        fi
        
        print_message "$GREEN" "Firmware copied to keyboard. Waiting for it to reboot..."
    elif [[ "$FLASH_METHOD" == "avrdude" ]]; then
        # Check if avrdude is installed
        if ! check_avrdude; then
            print_message "$RED" "avrdude is required for flashing Corne keyboard."
            print_message "$YELLOW" "You can manually flash the firmware with QMK Toolbox instead."
            print_message "$YELLOW" "Firmware file is located at: $file"
            read -p "Press Enter once you've manually flashed the firmware..."
            return 0
        fi
        
        print_message "$YELLOW" "Put the $side half into bootloader mode by pressing the reset button."
        print_message "$YELLOW" "You have 5 seconds to press the reset button..."
        
        # Display a countdown
        for i in {5..1}; do
            print_message "$BLUE" "$i seconds remaining..."
            sleep 1
        done
        
        # Interactive port detection
        print_message "$BLUE" "Looking for keyboard in bootloader mode..."
        local port=""
        local keep_trying=true
        
        while $keep_trying; do
            # Try to detect the port
            for p in /dev/tty.usbmodem* /dev/tty.usbserial* /dev/cu.usbmodem*; do
                if [[ -e "$p" ]]; then
                    port="$p"
                    break
                fi
            done
            
            if [[ -n "$port" ]]; then
                # Found a port
                print_message "$GREEN" "Found keyboard at $port"
                break
            else
                # No port found, ask user what to do
                print_message "$YELLOW" "No keyboard found in bootloader mode."
                print_message "$YELLOW" "Make sure the $side half is connected and in bootloader mode."
                print_message "$YELLOW" "Please press the reset button to put the keyboard in bootloader mode."
                print_message "$BLUE" "Options:"
                print_message "$BLUE" "  [r] - Retry detecting the keyboard"
                print_message "$BLUE" "  [s] - Skip flashing this half (not recommended)"
                print_message "$BLUE" "  [m] - Manually flash with another tool"
                print_message "$BLUE" "  [q] - Quit"
                read -p "What would you like to do? [r/s/m/q]: " choice
                
                case "$choice" in
                    r|R) 
                        print_message "$BLUE" "Retrying..."
                        ;;
                    s|S)
                        print_message "$YELLOW" "Skipping $side half. Your keyboard may not work correctly."
                        return 0
                        ;;
                    m|M)
                        print_message "$YELLOW" "You can manually flash with QMK Toolbox using: $file"
                        print_message "$YELLOW" "Or use avrdude directly with: avrdude -p atmega32u4 -c avr109 -P /path/to/port -U flash:w:$file:i"
                        read -p "Press Enter once you've manually flashed the firmware..."
                        return 0
                        ;;
                    q|Q)
                        print_message "$RED" "Quitting at user request."
                        exit 1
                        ;;
                    *)
                        print_message "$YELLOW" "Invalid choice. Retrying..."
                        ;;
                esac
            fi
        done
        
        print_message "$GREEN" "Found keyboard at $port"
        print_message "$BLUE" "Flashing with avrdude..."
        
        # Using avrdude to flash
        if ! avrdude -p atmega32u4 -c avr109 -P "$port" -U flash:w:"$file":i; then
            print_message "$RED" "Failed to flash with avrdude."
            print_message "$YELLOW" "You can try again or use QMK Toolbox to flash: $file"
            read -p "Press Enter once you've manually flashed the firmware..."
            return 1
        fi
        
        print_message "$GREEN" "Successfully flashed the $side half with avrdude."
        return 0
    else
        print_message "$RED" "Unknown flashing method: $FLASH_METHOD"
        return 1
    fi
    
    # Wait for the keyboard to disconnect
    local count=0
    while [[ -d "$MOUNT_POINT" && $count -lt $KEYBOARD_REBOOT_TIMEOUT ]]; do
        sleep 1
        ((count++))
        
        # Only show a message every 5 seconds to avoid too much output
        if (( count % 5 == 0 )); then
            print_message "$YELLOW" "Still waiting for keyboard to reboot... (${count}s)"
        fi
    done
    
    # Check if timed out or successfully disconnected
    if [[ $count -ge $KEYBOARD_REBOOT_TIMEOUT ]]; then
        print_message "$YELLOW" "Keyboard didn't disconnect within timeout, but firmware may still be flashed."
    else
        print_message "$GREEN" "$side half flashed successfully!"
    fi
    
    # Add a small delay to ensure the keyboard has time to reboot
    sleep 1
    
    return 0
}

# Function to watch the workflow run
watch_workflow() {
    local status
    status=$(get_workflow_status)
    
    if [[ "$status" == "completed" ]]; then
        local conclusion
        conclusion=$(get_workflow_conclusion)
        
        if [[ "$conclusion" == "success" ]]; then
            print_message "$GREEN" "Latest workflow run completed successfully."
            return 0
        else
            print_message "$RED" "Latest workflow run failed with conclusion: $conclusion"
            return 1
        fi
    fi
    
    print_message "$BLUE" "Watching the latest workflow run..."
    if ! gh run watch --repo "$REPO"; then
        print_message "$RED" "Workflow run failed."
        return 1
    fi
    
    # Double-check the conclusion after watching
    local conclusion
    conclusion=$(get_workflow_conclusion)
    
    if [[ "$conclusion" == "success" ]]; then
        print_message "$GREEN" "Workflow run completed successfully."
        return 0
    else
        print_message "$RED" "Workflow run failed with conclusion: $conclusion"
        return 1
    fi
}

# Function to clean up
cleanup() {
    local tmp_dir=$1
    if [[ -n "$tmp_dir" && -d "$tmp_dir" ]]; then
        rm -rf "$tmp_dir"
    fi
}

# Main function
main() {
    # Skip the first argument if it's a keyboard name
    local options=()
    if [[ "$1" == "charybdis" || "$1" == "ch" || "$1" == "corne" || "$1" == "crkbd" || "$1" == "co" ]]; then
        shift # Skip the keyboard argument since we've already processed it
    fi
    
    # Remaining arguments are options
    options=("$@")
    
    print_message "$BLUE" "Starting keyboard firmware flashing process for $KEYBOARD..."
    
    # Check dependencies
    check_gh_cli
    check_op_cli
    
    # Authenticate with GitHub
    authenticate
    
    # Watch the workflow run if needed
    if [[ "${options[0]}" != "--latest-successful" ]]; then
        if ! watch_workflow; then
            print_message "$RED" "Exiting due to workflow failure."
            exit 1
        fi
    else
        print_message "$YELLOW" "Using latest successful build as requested."
        
        # If using latest successful, we need to find and use the latest successful run
        local run_id
        run_id=$(gh run list --repo "$REPO" --status success --limit 1 --json databaseId -q '.[0].databaseId')
        
        if [[ -z "$run_id" ]]; then
            print_message "$RED" "No successful workflow runs found."
            exit 1
        fi
        
        print_message "$GREEN" "Found successful workflow run: $run_id"
    fi
    
    # Download the firmware - using global variables
    FIRMWARE_PATH=""
    TMP_DIR=""
    
    # Pass the run_id if we're using latest successful
    if [[ "${options[0]}" == "--latest-successful" ]]; then
        get_firmware "$run_id"
    else
        get_firmware
    fi
    
    if [[ ! -f "$FIRMWARE_PATH" ]]; then
        print_message "$RED" "Firmware file does not exist at path: $FIRMWARE_PATH"
        exit 1
    fi
    
    # Flash process with user interaction
    print_message "$BLUE" "Starting the flashing process..."
    print_message "$YELLOW" "This script will help you flash both halves of your keyboard."
    
    # Different processes based on keyboard type
    if [[ "$FLASH_METHOD" == "copy" ]]; then
        # For RP2040-based keyboards like Charybdis
        
        # Flash left half
        if ! flash_firmware "$FIRMWARE_PATH" "LEFT"; then
            print_message "$RED" "Failed to flash LEFT half."
            cleanup "$TMP_DIR"
            exit 1
        fi
        
        print_message "$YELLOW" "Left half flashed successfully!"
        print_message "$YELLOW" "Now prepare the RIGHT half of your keyboard."
        sleep $DELAY_BETWEEN_HALVES
        
        # Flash right half
        if ! flash_firmware "$FIRMWARE_PATH" "RIGHT"; then
            print_message "$RED" "Failed to flash RIGHT half."
            cleanup "$TMP_DIR"
            exit 1
        fi
    elif [[ "$FLASH_METHOD" == "avrdude" ]]; then
        # For AVR-based keyboards like Corne that use avrdude
        
        # Flash left half
        print_message "$YELLOW" "We'll flash the LEFT half first."
        if ! flash_firmware "$FIRMWARE_PATH" "LEFT"; then
            print_message "$RED" "Failed to flash LEFT half."
            cleanup "$TMP_DIR"
            exit 1
        fi
        
        print_message "$YELLOW" "Left half flashed successfully!"
        print_message "$YELLOW" "Now we'll flash the RIGHT half."
        sleep $DELAY_BETWEEN_HALVES
        
        # Flash right half
        if ! flash_firmware "$FIRMWARE_PATH" "RIGHT"; then
            print_message "$RED" "Failed to flash RIGHT half."
            cleanup "$TMP_DIR"
            exit 1
        fi
    else
        print_message "$RED" "Unsupported flashing method: $FLASH_METHOD"
        cleanup "$TMP_DIR"
        exit 1
    fi
    
    # All done
    print_message "$GREEN" "Both halves of your keyboard have been flashed successfully!"
    print_message "$GREEN" "Your keyboard is ready to use."
    
    # Clean up
    cleanup "$TMP_DIR"
}

# Parse command line arguments
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    echo -e "${BLUE}Keyboard Firmware Flasher${NC}"
    echo -e "A tool to download and flash firmware for QMK split keyboards"
    echo ""
    echo -e "${YELLOW}Usage:${NC} $0 [keyboard] [options]"
    echo ""
    echo -e "${YELLOW}Keyboards:${NC}"
    echo "  charybdis, ch         Flash Charybdis 3x6 keyboard (default)"
    echo "  corne, crkbd, co      Flash Corne v3 keyboard"
    echo ""
    echo -e "${YELLOW}Options:${NC}"
    echo "  --latest-successful   Use the latest successful build regardless of current workflow status"
    echo "  --no-color            Disable colored output"
    echo "  --help, -h            Show this help message"
    echo ""
    echo -e "${YELLOW}Prerequisites:${NC}"
    echo "  - GitHub CLI (gh) must be installed and authenticated"
    echo "  - 1Password CLI (op) is recommended for secure token retrieval"
    echo "  - QMK firmware must be built using GitHub Actions"
    echo ""
    echo -e "${YELLOW}Instructions:${NC}"
    echo "  1. Run this script after pushing changes to your keyboard firmware"
    echo "  2. The script will download the latest firmware build"
    echo "  3. Follow the prompts to enter bootloader mode for each half"
    echo "  4. For Charybdis keyboards:"
    echo "     - Put your keyboard in bootloader mode by pressing the reset button twice quickly"
    echo "  5. For Corne keyboards:"
    echo "     - The script will use avrdude to flash the firmware"
    echo "     - You'll need to press the reset button when prompted"
    echo "     - Make sure avrdude is installed: brew install avrdude"
    echo ""
    echo -e "${YELLOW}Troubleshooting:${NC}"
    echo "  - If no firmware is found, check your GitHub workflow"
    echo "  - If keyboard is not detected, ensure it's in bootloader mode"
    echo "  - For authentication issues, run 'gh auth login' first"
    echo ""
    exit 0
fi

# Process keyboard selection if present
if [[ "$1" == "charybdis" || "$1" == "ch" || "$1" == "corne" || "$1" == "crkbd" || "$1" == "co" ]]; then
    # Set the canonical keyboard name
    case "$1" in
      "charybdis"|"ch")
        KEYBOARD="charybdis"
        ;;
      "corne"|"crkbd"|"co")
        KEYBOARD="corne"
        ;;
    esac
    shift
fi

# Disable colors if requested
if [[ "$1" == "--no-color" || "$2" == "--no-color" ]]; then
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

# Run the main function
main "$@"
