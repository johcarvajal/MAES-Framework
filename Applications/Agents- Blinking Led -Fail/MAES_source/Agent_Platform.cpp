/*
 * maes.cpp
 *
 *  Created on: 14 May 2017
 *      Author: Carmen Chan-Zheng
 */
#include "maes.h"

namespace MAES{
/*********************************************************************************************
*
*                                  Class: Agent_Platform
*
**********************************************************************************************
**********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name){
        agentAMS.agent.agent_name=name;
        ptr_cond=&cond;
        subscribers=0;
        for (int i=0;i<AGENT_LIST_SIZE;i++){
           Agent_Handle[i]=(Agent_AID) NULL;
           Task_setEnv(Agent_Handle[i],NULL);
        }
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: Agent_Platform Constructor
* Comment: Initialize list of agents task handle and its environment to NULL.
*          This constructor is used if user has their own conditions
**********************************************************************************************/
    Agent_Platform::Agent_Platform(String name,USER_DEF_COND*user_cond){
        agentAMS.agent.agent_name=name;
        ptr_cond=user_cond;
        subscribers=0;
        for (int i=0;i<AGENT_LIST_SIZE;i++){
           Agent_Handle[i]=(Agent_AID) NULL;
           Task_setEnv(Agent_Handle[i],NULL);
        }

    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: bool init();
* Return: Boolean
* Comment: Create AMS task with default stack of 4096. Can only deployed in Main
***********************************************************************************************/
    bool Agent_Platform::boot(){
        if (Task_self()==NULL){
            Agent_AID temp;

            Mailbox_Params mbxParams;
            Task_Params taskParams;

            /*Creating mailbox*/
            Mailbox_Params_init(&mbxParams);
            agentAMS.agent.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=&task_stack;
            taskParams.stackSize = 4096;
            taskParams.priority = Task_numPriorities-1;//Assigning max priority
            taskParams.instance->name= "AMS";
            taskParams.env=&agentAMS;
            taskParams.arg0=(UArg)this;
            taskParams.arg1=(UArg)ptr_cond;
            agentAMS.agent.aid= Task_create(AMS_task, &taskParams, NULL);
            agentAMS.agent.AP=agentAMS.agent.aid;

            /*Initializing all the previously created task*/
            if (agentAMS.agent.aid!=NULL){
                temp=Task_Object_first();
                while (temp!=NULL && temp!=agentAMS.agent.aid!=NULL){
                    register_agent(temp);
                    temp=Task_Object_next(temp);
                }
                return NO_ERROR;
            }
            else {
                System_abort("AP init failed");
                return INVALID;
            }
        }
        else return INVALID;
   }
/**********************************************************************************************
* Class: Agent_Platform
* Function: void init_agent(Agent a, Task_FuncPtr behaviour, Agent_AID &aid);
* Return: NULL
* Comment: Creates the task and mailbox handle for the agent object.  Only can be called from
*          main.
*          Init to -1, only when registered and deployed will work
*          Agent a: Agent to be initialized
*          Task_FuncPtr behaviour: Behaviour to be assigned to the agent
*          Agent_AID &aid: Return the aid of the Agent.
**********************************************************************************************/
    void Agent_Platform::agent_init(Agent &a, Task_FuncPtr behaviour){
        if (Task_self()==NULL){
            Task_Params taskParams;
            Mailbox_Params mbxParams;

            /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
            Mailbox_Params_init(&mbxParams);
            a.agent.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

             /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=a.resources.stack;
            taskParams.stackSize =a.resources.stackSize;
            taskParams.priority = -1;
            taskParams.instance->name=a.agent.agent_name; //To do: take that out to optimize
            taskParams.env=(xdc_Ptr) &a;
            a.agent.aid = Task_create(behaviour, &taskParams, NULL);
        }

       // else aid=NULL;

    }
/**********************************************************************************************
* Class: Agent_Platform
* Function: void agent_init(Agent &a, Task_FuncPtr behaviour, UArg arg0, UArg arg1,Agent_AID &aid)* Return: NULL
* Comment: Creates the task and mailbox handle for the agent object.  Only can be called from
*          main.
*          Agent a: Agent to be initialized
*          Task_FuncPtr behaviour: Behaviour to be assigned to the agent
*          UArg arg0, arg1: Arguments to be passed to the behaviour
*          Agent_AID &aid: Return the aid of the Agent.
**********************************************************************************************/
    void Agent_Platform::agent_init(Agent &a, Task_FuncPtr behaviour,UArg arg0, UArg arg1){
        if (Task_self()==NULL){
            Task_Params taskParams;
            Mailbox_Params mbxParams;

            /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
            Mailbox_Params_init(&mbxParams);
            a.agent.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=a.resources.stack;
            taskParams.stackSize =a.resources.stackSize;
            taskParams.priority = -1;
            taskParams.instance->name=a.agent.agent_name; //To do: take that out to optimize
            taskParams.env=(xdc_Ptr) &a;
            taskParams.arg0= arg0;
            taskParams.arg1= arg1;
            a.agent.aid = Task_create(behaviour, &taskParams, NULL);
        }

    }
/**********************************************************************************************
* Class: Agent_Platform
* Function:  bool search(Agent aid);
* Return: Bool
* Comment: search agent in Platform by agent aid
**********************************************************************************************/
    bool Agent_Platform::agent_search(Agent_AID aid){
        for(int i=0;i<subscribers;i++){
            if (Agent_Handle[i]==aid) return true;
        }
        return false;
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: void agent_wait (uint32 ticks)
* Return type: NULL
* Comments: When called within agent's function it will make agent sleeps defined ticks
*********************************************************************************************/
    void Agent_Platform::agent_wait(Uint32 ticks){
        Task_sleep(ticks);
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: void agent_yield()
* Return type: NULL
* Comments: It yields the processor to another readied agent of equal priority.
*           It lower priorities are on readied, the current task won't be preempted
*********************************************************************************************/
    void Agent_Platform::agent_yield(){
        Task_yield();
    }
/*********************************************************************************************
* Class:Agent_Platform
* Function: get_running_agent()
* Return type: NULL
* Comments: Returns aid of current running agent
*********************************************************************************************/
    Agent_AID Agent_Platform::get_running_agent(){
        return Task_self();
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function:void get_state
* Return:  NULL
* Comment: get running mode of agent
**********************************************************************************************/
        AGENT_MODE Agent_Platform:: get_state(Agent_AID aid){
         if(agent_search(aid)){
             Task_Mode mode;
             mode=Task_getMode(aid);

             switch(mode){
             case Task_Mode_READY:
                 return ACTIVE;

             case Task_Mode_BLOCKED:
                 return WAITING;

             case Task_Mode_INACTIVE:
                 return SUSPENDED;

             case Task_Mode_TERMINATED:
                 return TERMINATED;

             default: return NO_MODE;

             }
         }
         else return NO_MODE;
        }

/*********************************************************************************************
* Class: Agent_Platform
* Function: AP_Description* get_Agent_description(Agent aid);
* Return: Agent_info
* Comment: Returns description of an agent. Returns copy instead of pointer since pointer
*          can override the information.
**********************************************************************************************/
    Agent_info Agent_Platform::get_Agent_description(Agent_AID aid){
        Agent *description;
        description=(Agent *)Task_getEnv(aid);
        return description->agent;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function: get_AP()
* Comment: Get information of Agent Platform.
**********************************************************************************************/
    AP_Description Agent_Platform::get_AP_description(){
        AP_Description description;
        description.AMS_AID=agentAMS.agent.aid;
        description.AP_name=agentAMS.agent.agent_name;
        description.subscribers=subscribers;
        return description;
    }

/*********************************************************************************************
* Class: Agent_Platform
* Function: int register_agent(Agent aid)
* Comment: Register agent to the platform only if
*          it is unique by agent's task. Only can be done by AMS or Main
**********************************************************************************************/
    ERROR_CODE Agent_Platform::register_agent(Agent_AID aid){
        if (aid==NULL) return HANDLE_NULL;
        else if (Task_self()==NULL || Task_getPri(Task_self())==Task_numPriorities-1){
            if (!agent_search(aid)){
                if(subscribers<AGENT_LIST_SIZE){
                    Agent *description;
                    description=(Agent*)Task_getEnv(aid);
                    description->agent.AP=agentAMS.agent.aid;
                    Agent_Handle[subscribers]=aid;
                    subscribers++;
                    Task_setPri(aid, description->agent.priority);
                    return NO_ERROR;
                }
                else return LIST_FULL;
            }
            else return DUPLICATED;
        }

        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function:  int deregister_agent(Agent aid);
* Comment: Deregister agent on the platform by handle. It searches inside of the list, when found,
*          the rest of the list is shifted to the right and the agent is removed.
**********************************************************************************************/
    ERROR_CODE Agent_Platform::deregister_agent(Agent_AID aid){
        if(Task_getPri(Task_self())==Task_numPriorities-1){
             int i=0;
            while(i<AGENT_LIST_SIZE){
                if(Agent_Handle[i]==aid){
                    Agent *description;
                    suspend_agent(aid);
                    description=(Agent *)Task_getEnv(aid);
                    description->agent.AP=NULL;

                    while(i<AGENT_LIST_SIZE-1){
                      Agent_Handle[i]=Agent_Handle[i+1];
                    i++;
                    }
                    Agent_Handle[AGENT_LIST_SIZE-1]=NULL;
                    subscribers--;
                    break;
                }
                    i++;
            }

            if (i==AGENT_LIST_SIZE) return NOT_FOUND;
            else return NO_ERROR;
        }

        else return INVALID;
   }
/*********************************************************************************************
* Class: Agent_Platform
* Function:  int kill_agent(Agent aid);
* Return: int
* Comment: Kill agent on the platform. It deregisters the agent first then it delete the
*           handles by agent's handle
**********************************************************************************************/
    ERROR_CODE Agent_Platform::kill_agent(Agent_AID aid){
        if(Task_getPri(Task_self())==Task_numPriorities-1){
            ERROR_CODE error;
            error=deregister_agent(aid);

            if(error==NO_ERROR){
                Agent *description;
                Mailbox_Handle m;

                /*Setting Agent's object aid to NULL*/
                description=(Agent*)Task_getEnv(aid);
                description->agent.aid=NULL;
                /*Deleting Mailbox and Task*/
                m=description->agent.mailbox_handle;
                Task_delete(&aid);
                Mailbox_delete(&m);
            }
            return error;
        }
        else return INVALID;
    }
/*********************************************************************************************
* Class: Agent_Platform
* Function:void suspend_agent(Agent aid)
* Return:  NULL
* Comment: suspend_agent Agent. Set it to inactive by setting priority to -1
**********************************************************************************************/
    ERROR_CODE Agent_Platform::suspend_agent(Agent_AID aid){
        if(Task_getPri(Task_self())==Task_numPriorities-1){
            if(agent_search(aid)) {
                Task_setPri(aid, -1);
                return NO_ERROR;
            }
            else return NOT_FOUND;
        }
        else return INVALID;
   }

/*********************************************************************************************
* Class: Agent_Platform
* Function:void restore(Agent agent)
* Return:  NULL
* Comment: Restore Agent.
**********************************************************************************************/
    ERROR_CODE Agent_Platform::resume_agent(Agent_AID aid){
        if(Task_getPri(Task_self())==Task_numPriorities-1){
            if(agent_search(aid)) {
                Agent* description;
                description=(Agent*)Task_getEnv(aid);
                Task_setPri(aid,description->agent.priority);
                return NO_ERROR;
            }
            else return NOT_FOUND;
        }
        else return INVALID;
   }
/*********************************************************************************************
* Class: Agent_Platform
* Function: restart(Agent_AID aid)
* Return: int
* Comment: Delete the agent and re-create it. Only AMS agent can perform the task.
**********************************************************************************************/
    void Agent_Platform::restart(Agent_AID aid){

        if(Task_getPri(Task_self())==Task_numPriorities-1){
            Agent *a;
            UArg arg0, arg1;
            Task_FuncPtr behaviour;

            /*Obtaining agent information to restart*/
            a= (Agent*) Task_getEnv(aid);
            behaviour = Task_getFunc(aid, &arg0, &arg1);


            Mailbox_Handle m;

            /*Deleting Mailbox and Task*/
            m=a->agent.mailbox_handle;
            Task_delete(&aid);
            Mailbox_delete(&m);
            Task_Params taskParams;
            Mailbox_Params mbxParams;

            /*Creating mailbox: Msg size is 20 and default queue size is set to 3*/
            Mailbox_Params_init(&mbxParams);
            a->agent.mailbox_handle= Mailbox_create(20,8,&mbxParams,NULL);

            /*Creating task*/
            Task_Params_init(&taskParams);
            taskParams.stack=a->resources.stack;
            taskParams.stackSize =a->resources.stackSize;
            taskParams.priority = a->agent.priority;
            taskParams.instance->name="test";//a->agent.agent_name; //To do: take that out to optimize
            taskParams.env=(xdc_Ptr) a;
            a->agent.aid = Task_create(behaviour, &taskParams, NULL);
        }

    }
/*********************************************************************************************
* Function: AMS_task
* Return: NULL
* Comment: AMS task description.
*          No visible outside of namespace. For internal use only
**********************************************************************************************/
    namespace{
        void AMS_task(UArg arg0, UArg arg1){
            Agent_Platform *services=(Agent_Platform*)arg0;
            USER_DEF_COND *cond= (USER_DEF_COND*)arg1;
            Agent_Msg msg;
//            REQUEST_ACTION request;
            int error_msg=0;
            while(1){
                    msg.receive(BIOS_WAIT_FOREVER);
                    if (msg.get_msg_type()==REQUEST){

                        if (strcmp(msg.get_msg_content(),"KILL")==0){
                            if(cond->kill_cond()){
                                error_msg=services->kill_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR) msg.set_msg_type(CONFIRM);
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }//Finish KILL Case

                        else if(strcmp(msg.get_msg_content(),"REGISTER")==0){
                            if(cond->register_cond()){
                                error_msg=services->register_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR) msg.set_msg_type(CONFIRM);
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }//Finish register case

                        else if(strcmp(msg.get_msg_content(),"DEREGISTER")==0){
                            if(cond->deregister_cond()){
                                error_msg=services->deregister_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR) msg.set_msg_type(CONFIRM);
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }//finish deregister

                        else if(strcmp(msg.get_msg_content(),"SUSPEND")==0){
                            if (cond->suspend_cond()){
                                error_msg=services->suspend_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR) msg.set_msg_type(CONFIRM);
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }//finish suspend

                        else if(strcmp(msg.get_msg_content(),"RESUME")==0){
                            if (cond->resume_cond()){
                                error_msg=services->resume_agent(msg.get_target_agent());
                                if(error_msg==NO_ERROR) msg.set_msg_type(CONFIRM);
                                else msg.set_msg_type(REFUSE);
                            }
                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }//finish resume

                        else if(strcmp(msg.get_msg_content(),"RESTART")==0){
//                            stop=Timestamp_get32();
//                            System_printf(" %d\n",stop-init);
//                            System_flush();
                            if(cond->restart_cond()) services->restart(msg.get_target_agent());

                            else msg.set_msg_type(REFUSE);

                            /*Respond*/
                            stop=Timestamp_get32();
                            System_printf(" %d\n",stop-init);
                            System_flush();
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);

                        }//finish restart

                        else{
                            msg.set_msg_type(NOT_UNDERSTOOD);
                            msg.send(msg.get_sender(),BIOS_NO_WAIT);
                        }

                    }//End if

                    else{
                        msg.set_msg_type(NOT_UNDERSTOOD);
                        msg.send(msg.get_sender(),BIOS_NO_WAIT);
                    }
                }//finish while
        }
    }
};
