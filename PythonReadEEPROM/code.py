import os

input_file_location = 'C:/Users/School/Desktop/PIB/BlueFinger/PythonReadEEPROM/DocumentEEPROM.txt'
output_file_location = 'C:/Users/School/Desktop/PIB/BlueFinger/PythonReadEEPROM/DocumentEEPROMDecimaal.txt'

# Open the input and output files
with open(input_file_location, 'r') as input_file, open(output_file_location, 'w') as output_file:
    # Iterate through each line in the input file
    for line in input_file:
        # Remove the first 9 and last 2 characters from the line
        edited_line = line[9:-3]
        # Create a space between every 2 characters
        edited_line = ' '.join(edited_line[i:i+2] for i in range(0, len(edited_line), 2))

        hex_values = edited_line.split()  # Split edited_line into a list of hex values
        decimal_values = []  # Initialize an empty list for decimal values

        # Convert each hex value to decimal and add it to the decimal_values list
        for hex_value in hex_values:
            decimal_value = int(hex_value, 16)
            decimal_values.append(decimal_value)

        # Join the decimal_values list into a string with spaces between each value
        decimal_string = ' '.join(str(decimal_value) for decimal_value in decimal_values)

        # Split the decimal_string into words
        words = decimal_string.strip().split(' ')
        new_line = ''
        
        # Create a new line with '.' between every other word
        for i, word in enumerate(words):
            if i % 2 == 0:
                new_line += word + '.'
            else:
                new_line += word + ' '
        
        # Write the new_line to the output file, with a newline character added at the end
        output_file.write(new_line[:-1] + '\n')