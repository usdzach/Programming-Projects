/* Author: Zach Fukuhara
 *
 * Description:
 * Compress.java is a program that compresses a user defined textfile and
 * outputs a compressed binary file with the extension .zzz
*/

import java.io.*;
public class Compress {
    public static void main(String[] args) {
        Compress("/Users/zachfukuhara/Library/COMP 285/src/TestFile.txt");
    }

    public static void Compress(String filename) {
        try {
            // Verify filename
            if (filename != null && filename.contains(".txt")) {

                // Create BufferedReader and DataOutputStream
                BufferedReader buf = new BufferedReader(new FileReader(filename));
                DataOutputStream output = new DataOutputStream(new FileOutputStream(filename + ".zzz"));


//---------------------------------------------------INITIALIZE DICTIONARY---------------------------------------------------

                // Declare a constant for the table size and the ascii counter variable
                final int TABLESIZE = 17;
                int asciiCount = 32;
                int numDictItems;
                // Create an array of Linked Lists
                LinkedList[] dictionary = new LinkedList[TABLESIZE];

                // Initialize the linked lists in the array
                for (int i = 0; i < TABLESIZE; i++)
                    dictionary[i % TABLESIZE] = new LinkedList();

                // Insert in the nodes into the dictionary
                for (numDictItems = 0; numDictItems < 95; numDictItems++) // numDictItems < 95 b/c we have to include the 126th ascii value
                    dictionary[numDictItems % TABLESIZE].ListInsertAt(1, (short)asciiCount, Character.toString((char)asciiCount++));


                // Insert \t, \r, and \n into the dictionary
                dictionary[10].ListInsertAt(1, (short)asciiCount++, "\t");
                dictionary[11].ListInsertAt(1, (short)asciiCount++, "\r");
                dictionary[12].ListInsertAt(1, (short)asciiCount++, "\n");
                numDictItems += 3;


//---------------------------------------------------COMPRESSION ALGORITHM---------------------------------------------------

                // STEP 1: Loop through input and find the longest prefix (P) of the uncoded part of the input file that is in the dictionary
                // STEP 2: Output the code
                // STEP 3: If there is a next character (C) in the input file, then assign (PC) the next code and insert it into the dictionary

                String prefix = "", nextChar;
                String binStr;
                boolean inDict = false;


                while (buf.ready()) { // while there is 1(+) character(s) in the buffer
                    nextChar = Character.toString((char)buf.read()); // Store next character from buffer in nextChar

                    for (int i = 0; i < TABLESIZE; i++) { // Check if the dictionary contains the prefix + nextChar
                        if (inDict = dictionary[i].ContainsKey(prefix + nextChar)) {
                            inDict = true;
                            break;
                        }
                    }

                    if (inDict) // If prefix + nextChar is in the dictionary, update longest known prefix
                        prefix += nextChar;
                    else // If prefix + nextChar is not in the dictionary, add it and output it.  Then update prefix.
                    {
                        dictionary[numDictItems++ % TABLESIZE].ListInsertAt(1, (short)asciiCount++, (prefix + nextChar));

                        for(int j = 0; j < TABLESIZE; j++) {
                            if(dictionary[j].getCode(prefix) != -1) {
                                output.writeShort((short)dictionary[j].getCode(prefix));
                            }
                        }
                        prefix = nextChar;
                    }
                }
                buf.close();
                output.close();
            }
        }
        catch (IOException e) {
            System.out.println("IOException e");
        }
    }
}
