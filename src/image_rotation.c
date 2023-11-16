#include "image_rotation.h"

//Global integer to indicate the length of the queue??
#define QUEUE_LENGTH 100
//Global integer to indicate the number of worker threads
int num_worker_threads;
//Global file pointer for writing to log file in worker??
FILE* log_file;
//Might be helpful to track the ID's of your threads in a global array
pthread_t worker_threads[MAX_THREADS]; // might be an int?
//What kind of locks will you need to make everything thread safe? [Hint you need multiple]
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
//What kind of CVs will you need  (i.e. queue full, queue empty) [Hint you need multiple]
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty = PTHREAD_COND_INITIALIZER;
//How will you track the requests globally between threads? How will you ensure this is thread safe?
pthread_cond_t processor_done = PTHREAD_COND_INITIALIZER;
//How will you track which index in the request queue to remove next?
int next_index_in_queue = 0;
//How will you update and utilize the current number of requests in the request queue?
int number_of_requests = 0;
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
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char * file_name){
    fprintf(log_file, "[%d][%d][%s]\n", threadId, requestNumber, file_name); // should look like [8][5][./img/30.png]
    printf("[%d][%d][%s]\n", threadId, requestNumber, file_name);
    fflush(stdout);
}


/*

    1: The processing function takes a void* argument called args. It is expected to be a pointer to a structure processing_args_t 
    that contains information necessary for processing.

    2: The processing thread need to traverse a given dictionary and add its files into the shared queue while maintaining synchronization using lock and unlock. 

    3: The processing thread should pthread_cond_signal/broadcast once it finish the traversing to wake the worker up from their wait.

    4: The processing thread will block(pthread_cond_wait) for a condition variable until the workers are done with the processing of the requests and the queue is empty.

    5: The processing thread will cross check if the condition from step 4 is met and it will signal to the worker to exit and it will exit.

*/
request_t reqlist[QUEUE_LENGTH];
int index_counter = 0;

void *processing(void *args) {
    processing_args_t *pargs = (processing_args_t *) args;
    DIR *dir = opendir(pargs->input_dir);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Failed to open directory");
        exit(1);
    }
    
    pthread_mutex_lock(&queue_lock);

    while ((entry = readdir(dir)) != NULL) { 
        
        if ((strcmp((char *)entry->d_name, ".") == 0) || (strcmp((char *)entry->d_name, "..") == 0)) {
            continue;
        }

        const char* file_ext = strrchr(entry->d_name, '.'); //getting file extension

        if (file_ext == NULL || file_ext[0] == '\0') {
            pthread_mutex_unlock(&queue_lock);
            continue;
        }

        if (strcmp(file_ext, ".png") == 0) {
            reqlist[index_counter].file_name = entry->d_name;
            reqlist[index_counter].rotation_angle = pargs->rotation_angle;
            index_counter++;
            number_of_requests++;
        }
        // send signal to worker

        if (number_of_requests == 1) {
            pthread_cond_signal(&queue_full);
        }
    }

    pthread_mutex_unlock(&queue_lock);

    printf("numreq: %d\n", number_of_requests);

    pthread_mutex_lock(&queue_lock);
    pthread_cond_signal(&processor_done);
    pthread_mutex_unlock(&queue_lock);

    pthread_exit(NULL);
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
    worker_args_t *wargs = (worker_args_t *) args;
    
        /*
            Stbi_load takes:
                A file name, int pointer for width, height, and bpp

        */
    int width;
    int height;                            
    int bpp;                                                        

    while (1) {
        pthread_mutex_lock(&queue_lock);
        while (number_of_requests == 0) {
            pthread_cond_wait(&queue_full,  &queue_lock);
            
        }
        
        uint8_t* image_result = stbi_load(reqlist[next_index_in_queue].file_name, &width, &height, &bpp,  CHANNEL_NUM);   

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
        linear_to_image(image_result, img_matrix, width, height);
        

        ////TODO: you should be ready to call flip_left_to-_right or flip_upside_down depends on the angle(Should just be 180 or 270)
        //both take image matrix from linear_to_image, and result_matrix to store data, and width and height.
        //Hint figure out which function you will call. 
        //flip_left_to_right(img_matrix, result_matrix, width, height); or flip_upside_down(img_matrix, result_matrix ,width, height);
        if (reqlist[next_index_in_queue].rotation_angle == 180) {
            flip_left_to_right(img_matrix, result_matrix, width, height);
        }
        else {
            flip_upside_down(img_matrix, result_matrix ,width, height);
        }

        
        
        //uint8_t* img_array = NULL; ///Hint malloc using sizeof(uint8_t) * width * height
        uint8_t* img_array = (uint8_t *)malloc(sizeof(uint8_t) * width * height);


        ///TODO: you should be ready to call flatten_mat function, using result_matrix
        //img_arry and width and height; 
        flatten_mat(result_matrix, img_array, width, height);


        ///TODO: You should be ready to call stbi_write_png using:
        //New path to where you wanna save the file,
        //Width
        //height
        //img_array
        //width*CHANNEL_NUM
        char full_path[BUFF_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", wargs->input_dir, get_filename_from_path(reqlist[next_index_in_queue].file_name));
        stbi_write_png(full_path, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM);

        wargs->requests_processed++;
        number_of_requests--;
        next_index_in_queue++;
        

        for (int i = 0; i < width; i++) {
            free(result_matrix[i]);
            free(img_matrix[i]);
        }
        free(result_matrix);
        free(img_matrix);
        free(img_array);
        
        log_pretty_print(log_file, wargs->threadId, wargs->requests_processed, full_path);

        pthread_cond_signal(&queue_empty); // Signal that the queue is not full
        pthread_cond_signal(&processor_done); // Signal that the worker is done

        pthread_mutex_unlock(&queue_lock);
    }

    pthread_exit(NULL);

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
    num_worker_threads = atoi(argv[3]);
    int rotation_angle = atoi(argv[4]);

    number_of_requests = 0;

    log_file = fopen(LOG_FILE_NAME, "w"); // open log_file
    if (log_file == NULL) {
        printf("Cannot open log_file\n");
        exit(-1);
    }

    processing_args_t processor_args = {input_dir, num_worker_threads, rotation_angle};

    pthread_t processor; // create single processor thread
    if (pthread_create(&processor, NULL, (void *)processing, &processor_args) != 0) {
        fprintf(stderr, "Error creating processing thread\n");
        exit(-1);
    }

    for (int i = 0; i < num_worker_threads; i++) { // create n worker threads
        worker_args_t worker_args = {input_dir, output_dir, i, 0};
        if (pthread_create(&worker_threads[i], NULL, (void *)worker, &worker_args) != 0) {
            fprintf(stderr, "Error creating a worker thread\n");
            exit(-1);
        }
    }

    pthread_join(processor, NULL); // joining processor thread
    for (int j = 0; j < num_worker_threads; j++) { // joining worker threads
        pthread_join(worker_threads[j], NULL);
    }

    // for (int i=0; i < queue_length; i++) {
    //     printf("file name: %s\n", reqlist[i].file_name);
    //     printf("rotation angle: %d\n", reqlist[i].rotation_angle);
    // }

    fclose(log_file);
    return 0;
}
