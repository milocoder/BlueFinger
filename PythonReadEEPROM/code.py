input_file_location = 'C:/Users/johan/OneDrive/Bureaublad/PIB_code/BlueFinger/PythonReadEEPROM/DocumentEEPROM.txt'
output_file_location = 'C:/Users/johan/OneDrive/Bureaublad/PIB_code/BlueFinger/PythonReadEEPROM/DocumentBeter.txt'
# Open the input and output files
with open(input_file_location, 'r') as input_file, open(output_file_location, 'w') as output_file:
    # Iterate through each line in the input file
    for line in input_file:
        # Remove the first 9 and last 2 characters from the line
        edited_line = line[9:-3]
        # Create a space between every 2 characters
        edited_line = ' '.join(edited_line[i:i+2] for i in range(0, len(edited_line), 2))
        hex_values = edited_line.split()
        decimal_values = [int(hex_value, 16) for hex_value in hex_values]
        # Write to output file
        output_file.write(' '.join(str(decimal_value) for decimal_value in decimal_values) + '\n')
