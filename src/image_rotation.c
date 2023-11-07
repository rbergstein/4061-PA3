#include "image_rotation.h"
 
typedef struct ProcessorArgument {
    char* input_dir;
    int num_worker_threads;
    int rotation_angle;
} processArgs;

typedef struct WorkerArgument {
    char* threadID;
} workerArgs;

//Global integer to indicate the length of the queue??
int queue_length;
//Global integer to indicate the number of worker threads
int num_worker_threads;
//Global file pointer for writing to log file in worker??
FILE* log_file;
//Might be helpful to track the ID's of your threads in a global array
pthread_t threads_arr[MAX_THREADS]; // might be an int?
//What kind of locks will you need to make everything thread safe? [Hint you need multiple]
//What kind of CVs will you need  (i.e. queue full, queue empty) [Hint you need multiple]
//How will you track the requests globally between threads? How will you ensure this is thread safe?
//How will you track which index in the request queue to remove next?
//How will you update and utilize the current number of requests in the request queue?
//How will you track the p_thread's that you create for workers?
//How will you know where to insert the next request received into the request queue?


/*
    The Function takes:
    to_write: A file pointer of where to write the logs. 
    requestNumber: the request number that the thread just finished.
    file_name: the name of the file that just got processed. 

    The function output: 
    it should output the threadId, requestNumber, file_name into the logfile and stdout.
*/
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char * file_name){ // not needed for inter sub
   
}


/*

    1: The processing function takes a void* argument called args. It is expected to be a pointer to a structure processing_args_t 
    that contains information necessary for processing.

    2: The processing thread need to traverse a given dictionary and add its files into the shared queue while maintaining synchronization using lock and unlock. 

    3: The processing thread should pthread_cond_signal/broadcast once it finish the traversing to wake the worker up from their wait.

    4: The processing thread will block(pthread_cond_wait) for a condition variable until the workers are done with the processing of the requests and the queue is empty.

    5: The processing thread will cross check if the condition from step 4 is met and it will signal to the worker to exit and it will exit.

*/

void *processing(void *args) {
    processArgs *pargs = (processArgs *)args;
    DIR *dir = opendir(pargs->input_dir);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Failed to open directory");
        exit(1);
    }

    while((entry = readdir(dir)) != NULL){ 

    }


}

/*
    1: The worker threads takes an int ID as a parameter

    2: The Worker thread will block(pthread_cond_wait) for a condition variable that there is a requests in the queue. 

    3: The Worker threads will also block(pthread_cond_wait) once the queue is empty and wait for a signal to either exit or do work.

    4: The Worker thread will processes request from the queue while maintaining synchronization using lock and unlock. 

    5: The worker thread will write the data back to the given output dir as passed in main. 

    6:  The Worker thread will log the request from the queue while maintaining synchronization using lock and unlock.  

    8: Hint the worker thread should be in a While(1) loop since a worker thread can process multiple requests and It will have two while loops in total
        that is just a recommendation feel free to implement it your way :) 
    9: You may need different lock depending on the job.  

*/


void * worker(void *args) {


        /*
            Stbi_load takes:
                A file name, int pointer for width, height, and bpp

        */

       // uint8_t* image_result = stbi_load("??????","?????", "?????", "???????",  CHANNEL_NUM);
        

        uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
        uint8_t** img_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
        for(int i = 0; i < width; i++){
            result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
            img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
        }
        /*
        linear_to_image takes: 
            The image_result matrix from stbi_load
            An image matrix
            Width and height that were passed into stbi_load
        
        */
        //linear_to_image("??????", "????", "????", "????");
        

        ////TODO: you should be ready to call flip_left_to_right or flip_upside_down depends on the angle(Should just be 180 or 270)
        //both take image matrix from linear_to_image, and result_matrix to store data, and width and height.
        //Hint figure out which function you will call. 
        //flip_left_to_right(img_matrix, result_matrix, width, height); or flip_upside_down(img_matrix, result_matrix ,width, height);


        
        
        //uint8_t* img_array = NULL; ///Hint malloc using sizeof(uint8_t) * width * height
        uint8_t* img_array = (uint8_t *)malloc(sizeof(uint8_t) * width * height); // attempt at above TODO - Ryan


        ///TODO: you should be ready to call flatten_mat function, using result_matrix
        //img_arry and width and height; 
        //flatten_mat("??????", "??????", "????", "???????");


        ///TODO: You should be ready to call stbi_write_png using:
        //New path to where you wanna save the file,
        //Width
        //height
        //img_array
        //width*CHANNEL_NUM
       // stbi_write_png("??????", "?????", "??????", CHANNEL_NUM, "??????", "?????"*CHANNEL_NUM);
    

}

/*
    Main:
        Get the data you need from the command line argument 
        Open the logfile
        Create the threads needed
        Join on the created threads
        Clean any data if needed. 


*/

int main(int argc, char* argv[]) {
    if(argc != 5)
    {
        fprintf(stderr, "Usage: File Path to image dirctory, File path to output dirctory, number of worker thread, and Rotation angle\n");
    }
    
    ///TODO: 
    // image_rotation is made with "./image_rotation <input_dir> <output_dir> <number_threads> <rotation_angle>" via the writeup
    // For the intermediate submission, you only need to traverse a given directory and populating the request queue. 
    // Also, spawn N worker threads, print their threadID (Which you can pass in as parameters when creating the thread) and exit.

    char* input_dir = argv[1]; // used to create the single processing thread
    char* output_dir = argv[2]; // used for the rest of the N worker threads
    num_worker_threads = argv[3];
    int rotation_angle = argv[4];

    queue_length = 0;

    log_file = fopen("logfile.txt", "w"); // open log_file
    if (log_file == NULL) {
        printf("Cannot open log_file\n");
        exit(-1);
    }

    processArgs processor_args = {input_dir, &num_worker_threads, &rotation_angle};

    pthread_t processor; // create single processor thread
    if (pthread_create(&processor, NULL, (void *)processing, &processor_args) != 0) {
        fprintf(stderr, "Error creating processing thread\n");
        exit(-1);
    }
    
    pthread_t worker; // create n worker threads
    for (int i = 0; i < num_worker_threads; i++) {
        char threadID[100];
        sprintf(threadID, "Worker Thread ID: %d\n", i);
        workerArgs worker_arg = {threadID};
        if (pthread_create(&worker, NULL (void *)worker, &worker_arg) != 0) {
            fprintf(stderr, "Error creating a worker thread\n");
            exit(-1);
        }
    }

    pthread_join(processor, NULL);
    for (int i = 0; i < num_worker_threads; i++) {
        pthread_join(worker, NULL);
    }

    fclose(log_file);
}

