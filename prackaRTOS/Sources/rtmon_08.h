// =======================================================================
// rtmon.H
//
// Generic file for all platforms
//
// Defines prototypes of RTMON services and data structures
// Header for RTMON
// =======================================================================
#ifndef  RTMON_01A_H_09
    #define RTMON_01A_H_09


// RTMON_SMALL - use small RTMON data structures
// RTMON_SMALL - if defined, some variables in process and queue descriptor are
// 8 bits instead of 16 bits. This reduces the size or RAM required per process/queue
// but some limitations arrise, e.g. max stack size is 255, max time to delay or period
// is 255 etc.  see rtmon.c
// NOTE that you cannot define/undefine the RTMON_SMALL directive just in
// client program; You must rebuild RTMON lib after such change!
//#define RTMON_SMALL



// Error codes for RTMON services
#define         RTMON_OK            (0)		  // OK, no error	
#define         RERR_ID_PROC        (3)       // invalid process ID 
#define			    RERR_ST_PROC		    (4)		  // invalid service request for current state of the process, e.g. continue_p for process which is not delayed
#define         RERR_NO_MEM         (1)        // not enough memory - returns create_p if stack pool size or number of processes is too high
#define         RERR_NO_SUP         (9)       // invalid argument for service call, e.g. calling create_q with more than one message buffer 
#define         RERR_ID_QUEUE       (5)         // invalid queue ID
#define         RERR_FULL_Q         (7)     // queue is full (write_q)
#define         RERR_EMPTY_Q        (6)         // queue is empty     (read_q)
#define         RERR_WAIT_Q         (10)     // cannot wait for queue because another process is already waiting (read_q_w a write_q_w)

//
// Data structure for information about process (task descriptor)
// IDPROC
typedef struct 
{
        unsigned char ident;     // process ID (index into array of IDPROC structures)
        unsigned char no_ident;  // process ID negated 
        unsigned char prio;     // priority
        unsigned char   stat;   // state of the process
        void (*pfunc)(void);    // address of the process code (function)        
        unsigned int pstack;   //  address of the top of the stack (where SP points when stack is empty)               
        unsigned int akt_stack;		// current value of SP (current possition in the stack)
        
#ifdef  RTMON_SMALL
        unsigned char stack_size;     // size of the stack 
        unsigned char time_period;    // period in ticks when process is periodically run
        unsigned char time_to_start;  // time to the next start for periodical process
        unsigned char time_to_continue;   // time to continue for delay_p		
#else        
        unsigned int stack_size;     // size of the stack 
        unsigned int time_period;    // period in ticks when process is periodically run
        unsigned int time_to_start;  // time to the next start for periodical process
        unsigned int time_to_continue;   // time to continue for delay_p		
#endif
} IDPROC;     

// fix: 2010-11-08 - no need for this ifdef; if the lib is built without queues then
// linker will report errors.
//#ifdef  RTMON_USE_QUEUES
//
// Data structure for queue
// IDQUEUE
typedef struct
{
        unsigned char ident;     // queue ID (index into array of IDQUEUE structures)
        unsigned char no_ident;  // negated    
        unsigned char l_msg;    // length of the message buffer
        unsigned char n_wait_buff;  // flag indicating if queue is full (1 = full, 0 = empty)
        char* buff;					// pointer to the buffer of the queue
        IDPROC*  pid_proc_msg;		// process waiting for message in this queue (pointer to its IDPROC) - read_q_w
        IDPROC* pid_proc_buff;      // process waiting for this queue to become empty (pointer to its IDPROC) - write_q_w
        
} IDQUEUE;
//#endif  // RTMON_USE_QUEUES


//
// Prototypes for RTMON services
//

/*******************************************************************
* char rtm_init( IDPROC** init_id );
*
* IDPROC** init_id;               // receives ID of the initialization process 
*
* PURPOSE : Initialize RTMON (data structures, create dummy and init process)
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_init(IDPROC** init_id);  

/*******************************************************************
* char rtm_end(IDPROC* init_id);
*
* IDPROC** init_id;  - the ID of the initialization process as 
*                     returned by rtm_init 
*
* PURPOSE : End RTMON 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_end(IDPROC* init_id);

// Support for small-size RTMON data structures
// NOTE that you cannot define/undefine the RTMON_SMALL directive just in
// client program; You must rebuild RTMON lib after such change!
// RTMON_SMALL is defined in rtmon_config.h
#ifdef RTMON_SMALL

//
// SMALL rtmon data size
//
char rtm_create_p(const char* pname, unsigned char prio, void(*pfunc)( ), unsigned char stack_size, IDPROC** proc_id );
char rtm_start_p(IDPROC* proc_id, unsigned char time_to_start, unsigned char time_period);
char rtm_delay_p(IDPROC* proc_id, unsigned char time_to_delay);
char rtm_ch_period_p(IDPROC * proc_id, unsigned char time_period);

#else

//
// NORMAL rtmon data size
//
/*******************************************************************
* char rtm_create_p(const char* pname, unsigned char prio, void(*pfunc)( ), int stack_size, IDPROC** proc_id );
*
* const char* pname   - name of the process. NOT USED and not stored by RTMON 
* unsigned char prio  - priority of the process (1 thru 254)
* void(*pfunc)( )     - pointer to the code of the process 
* int stack_size      - stack size of the process
* IDPROC** proc_id    - receives the ID of the new process
*
* PURPOSE : create new process (task) in rtmon 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_create_p(const char* pname, unsigned char prio, void(*pfunc)( ), int stack_size, IDPROC** proc_id );

/*******************************************************************
* char rtm_start_p(IDPROC* proc_id, int time_to_start, int time_period);
*
* IDPROC* proc_id   - ID of the process to be started 
* int time_to_start - time delay (in ticks) for the start; 0 to start immediatelly
* int time_period   - period of automatic restart of the process, 0 to start only once.  
*
* PURPOSE : starts a process. 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_start_p(IDPROC* proc_id, int time_to_start, int time_period);

/*******************************************************************
* char rtm_delay_p(IDPROC* proc_id, int time_to_delay);
*
* IDPROC* proc_id   - ID of the process to suspend
* int time_to_delay - how long it shoudl be suspended in ticks. 0 = infinite
* 
* PURPOSE : suspend process execution for specified period of time
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_delay_p(IDPROC* proc_id, int time_to_delay);

/*******************************************************************
* char rtm_ch_period_p(IDPROC * proc_id, int time_period);
*
* IDPROC* proc_id   - ID of the process whose period is to be changed
* int time_period   - the new period for the process. If 0 the process will 
*                   no longer be periodically re-started by rtmon.
* 
* PURPOSE : change the period of automatic start of a process
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_ch_period_p(IDPROC * proc_id, int time_period);

#endif //  RTMON_SMALL

/*******************************************************************
* char rtm_continue_p(IDPROC * proc_id);
*
* IDPROC* proc_id   - ID of the process to be continued
* 
* PURPOSE : continue execution of a process previously delayed by rtm_delay_p call.
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_continue_p(IDPROC * proc_id);

/*******************************************************************
* char rtm_stop_p(IDPROC * proc_id );
*
* IDPROC* proc_id   - ID of the process to be stopped
* 
* PURPOSE : stop execution of given process. The process will no longer be scheduled
*         on CPU. Note that if it is periodical process it will be started again 
*         normally in the next period! 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_stop_p(IDPROC * proc_id );

/*******************************************************************
* char rtm_abort_p(IDPROC * proc_id );
*
* IDPROC* proc_id   - ID of the process to be aborted
* 
* PURPOSE : removes the process from list of rtmon processes
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES : reclaiming of the RAM space used by a process is not supported 
*       so this function has practically no use.    
*           
*/
char rtm_abort_p(IDPROC * proc_id );

// Queue functions are optional
//#ifdef  RTMON_USE_QUEUES
// fix: 2010-11-08 - no need for this ifdef; if the lib is built without queues then
// linker will report errors.


/*******************************************************************
* char rtm_create_q(const char* pname, char l_msg, char n_buff, IDQUEUE** pid_queue);
*
* const char* pname   - name of the queue. Not used and not stored by rtmon! 
* char l_msg          - lenght of the buffer for one message (size of the Q)
* char n_buff         - number of messages which can be placed into the Q. 
*                     THIS MUST BE 1. Only 1 message per queue is supported!
* IDQUEUE** pid_queue - received the ID of the new queue
* 
* PURPOSE : creates a queue 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES :   
*           
*/
char rtm_create_q(const char* pname, char l_msg, char n_buff, IDQUEUE** pid_queue);

/*******************************************************************
* char rtm_write_q(IDQUEUE*   pid_queue, void* pdata);
*
* IDQUEUE*  pid_queue   - ID of the queue to write data into  
* void* pdata           - pointer to the memory which contains source data 
* 
* PURPOSE : writes data into a queue 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES : rtmon reads the number of bytes N from the pdata location, where
*       N is the size of the queue defined in rtm_create_q param l_msg.   
*           
*/
char rtm_write_q(IDQUEUE*   pid_queue, void* pdata);
/*******************************************************************
* char rtm_write_q_w(IDQUEUE*   pid_queue, void* pdata);
*
* IDQUEUE*  pid_queue   - ID of the queue to write data into  
* void* pdata           - pointer to the memory which contains source data 
* 
* PURPOSE : writes data into a queue. Waits for the queue to become empty if it 
*         already contains a message.
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES : rtmon reads the number of bytes N from the pdata location, where
*       N is the size of the queue defined in rtm_create_q param l_msg.   
*           
*/
char rtm_write_q_w(IDQUEUE*   pid_queue, void* pdata);

/*******************************************************************
* char rtm_read_q(IDQUEUE*   pid_queue, void* pdata);
*
* IDQUEUE*  pid_queue   - ID of the queue to read data from  
* void* pdata           - pointer to the memory which revceives the data 
* 
* PURPOSE : reads data from a queue. 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES : rtmon writes the number of bytes N to the pdata location, where
*       N is the size of the queue defined in rtm_create_q param l_msg.   
*           
*/
char rtm_read_q(IDQUEUE*   pid_queue, void* pdata);
/*******************************************************************
* char rtm_read_q_w(IDQUEUE*   pid_queue, void* pdata);
*
* IDQUEUE*  pid_queue   - ID of the queue to read data from  
* void* pdata           - pointer to the memory which revceives the data 
* 
* PURPOSE : reads data from a queue. Waits for the queue to become full if it 
*         does not currently contain any message. 
*
* RETURN :  RTMON_OK (0) if successful or error code.
*                      
* NOTES : rtmon writes the number of bytes N to the pdata location, where
*       N is the size of the queue defined in rtm_create_q param l_msg.   
*           
*/
char rtm_read_q_w(IDQUEUE*   pid_queue, void* pdata);

//#endif  // RTMON_USE_QUEUES


/////////////////////////////////////////////////
#endif  // RTMON_01A_H_09
