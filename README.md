In this project, we first create a two-pointer for reading and writing, and both use mmap() function. 
    # mmap() is an effective than doing File* ptr.
After reading the file into the memory, we then use the for-loop to read each record(100) with its key into a pointer and use this pointer to do a quick parallel sort. In this quick sort, we implement two threads on each side, left and right divided by pivot, and do recursive quick-sort steps until used threads reach to maximum threads or there are fewer records to perform key ordering. Finally, calling a for-loop by doing write output to the file eventually and the program terminates.
