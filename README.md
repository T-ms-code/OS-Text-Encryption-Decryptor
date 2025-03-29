**Description**  
The project implements an encryptor/decryptor in C to process an input file containing words.  
The program maps the file into memory and utilizes multiple processes to apply random permutations to each word, storing the results in an output file.  
It can either encrypt words when provided with only the input file or decrypt them when supplied with an encrypted file and the corresponding permutations.    


**Used Concepts**   
1)Processes (fork, wait, etc.)  
2)Shared memory (shm_open, ftruncate, mmap etc.)      
3)Synchronization     
