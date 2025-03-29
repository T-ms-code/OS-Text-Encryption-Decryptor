**Description**  
An encoder/decoder (in C) should be implemented that takes an input file with different words.  
The program maps the input file into memory and starts several processes that will apply a random permutation to each word.  
The permutations will be written to an output file. The program can receive as an argument only the input file, in which case it will encrypt the words;  
or it can receive a file containing the encrypted words and the permutations used for encryption, in which case it will generate an output file with the decrypted words.  


**Used Concepts**   
1)Processes (fork, wait, etc.)  
2)Shared memory (shm_open, ftruncate, mmap etc.)      
3)Synchronization     
