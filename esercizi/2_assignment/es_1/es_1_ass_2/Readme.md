## Psuedo Code to read a binary file and count the number of occurences of each byte value

```cpp


1. Open the input binary file.
2. Check if the file is successfully opened.
   - If not, print an error message and exit the program.

3. Initialize a data structure (e.g., an array or a vector) to store byte occurrences.
   - The size of the data structure should be 256 to cover all possible byte values (0 to 255).
   - Initialize all elements to zero.

4. Read each byte from the file until the end of the file is reached.
   - For each byte read:
     - Increment the corresponding element in the data structure (byte occurrences).

5. Close the input file.

6. Open an output file to store the results.

7. Write the byte occurrences to the output file.
   - For each byte value from 0 to 255:
     - Write the byte value (in hexadecimal) and its occurrences to the output file.

8. Close the output file.

9. Optionally, display a success message.

10. Exit the program.

