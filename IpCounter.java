/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ipcounter;
import java.io.*;
import java.util.*;

/**
 *
 * @author nolan
 */
public class IpCounter {
public static void main(String [] args) {

        // The name of the file to open.
        String fileName = "cleanData1.txt";
        
        ArrayList<String> ar = new ArrayList<String>();
        HashSet noDupSet = new HashSet();
        // This will reference one line at a time
        String line = null;
 
        try {
            // FileReader reads text files in the default encoding.
            FileReader fileReader = 
                new FileReader(fileName);

            // Always wrap FileReader in BufferedReader.
            BufferedReader bufferedReader = 
                new BufferedReader(fileReader);

            while((line = bufferedReader.readLine()) != null) {
                //System.out.println(line.substring(0,12));
                
                if(line.substring(12).equals("-")||line.substring(11).equals("-")||line.substring(10).equals("-")){
                 //noDupSet.add(line.substring(0,9)+ "   ");
                 //ar.add(line.substring(0,9)+"   ");
                }else{
                 noDupSet.add(line.substring(0,12).replace("-",""));
                 ar.add(line.substring(0,12).replace("-",""));
                }
            }   
            PrintStream out = new PrintStream(new FileOutputStream("uniqueIPs.txt"));
            Iterator hashSetIterator = noDupSet.iterator();
            out.println("Number of IPs: "+ar.size());
            out.println("Unique Nodes: " +noDupSet.size());
            out.println("Unique: ");
            while(hashSetIterator.hasNext()){
                out.println(hashSetIterator.next());
            }


            out.close();
            System.out.println(ar.size());
            System.out.println(noDupSet.size());
            System.out.println(noDupSet);
            // Always close files.
            bufferedReader.close();         
        }
        catch(FileNotFoundException ex) {
            System.out.println(
                "Unable to open file '" + 
                fileName + "'");                
        }
        catch(IOException ex) {
            System.out.println(
                "Error reading file '" 
                + fileName + "'");                  
            // Or we could just do this: 
            // ex.printStackTrace();
        }
    }
}

