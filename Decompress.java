/* Author: Zach Fukuhara
 *
 * Description:
 * Decompress.java is a program that decompresses a user-defined compressed binary
 * file with the extension .zzz and outputs a textfile
*/

import java.io.*;
public class Decompress {
    public static void main(String[] args) {
        Decompress("TestFile.txt.zzz");
    }

    public static void Decompress(String filename)
    {
        try
        {
            if (filename != null && filename.contains(".zzz"))
            {
                DataInputStream input = new DataInputStream(new FileInputStream(filename));
                BufferedWriter buf = new BufferedWriter(new FileWriter("Output.txt"));


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


//--------------------------------------------------DECOMPRESSION ALGORITHM--------------------------------------------------

                short current = -1, previous = -1;
                String keyPrev = "", keyCur = "";
                boolean inDict = false;


                previous = input.readShort();

                for (int i = 0; i < TABLESIZE; i++) {
                    if (dictionary[i].getKey(previous) != null)
                    {
                        keyPrev = dictionary[i].getKey(previous);
                    }
                }

                buf.write(keyPrev, 0, keyPrev.length());

                while(input.available() > 0) { // while the dataInputStream has content remaining

                    current = input.readShort();

                    for (int k = 0; k < TABLESIZE; k++) { // Check if the dictionary contains current
                        if (inDict = dictionary[k].ContainsCode(current)) {
                            inDict = true;
                            break;
                        }
                    }

                    // Calculate the keys corresponding to [previous] and [current]
                    for (int j = 0; j < TABLESIZE; j++) {
                        if (dictionary[j].getKey(previous) != null)
                            keyPrev = dictionary[j].getKey(previous);
                        if (dictionary[j].getKey(current) != null)
                            keyCur = dictionary[j].getKey(current);
                    }

                    if (inDict) {
                        buf.write(keyCur, 0, keyCur.length());
                        dictionary[numDictItems++ % TABLESIZE].ListInsertAt(1, (short)asciiCount++, keyPrev + keyCur.substring(0, 1));
                    }
                    else {
                        buf.write(keyPrev + keyPrev.substring(0, 1), 0, keyPrev.length() + 1);
                        dictionary[numDictItems++ % TABLESIZE].ListInsertAt(1, (short)asciiCount++, keyPrev + keyPrev.substring(0, 1));
                    }
                    previous = current;
                }
                buf.close();
                input.close();
            }
        }
        catch (IOException ex) {
            System.out.println("EX");
        }
    }
}
