#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
struct Task;
int gcd(int,int);
int lcm(int,int);
int calculate_hyperperiod(const vector<Task>&);
bool checkFeasibility(vector<Task>&);
int findFirstSimultaneousRelease(const std::vector<Task>&);
void sortByExplicitPriority(vector<Task>&);
void sortByEarliestDeadline(vector<Task>&);
void sortByLeastLaxity(vector<Task>&);
void sortTasksByReleaseTime(vector<Task>&);
void readInputFile(const string&,vector<Task>&,vector<Task>&);
int getAlgorithmCode(string);
string getServerType(string);

struct Task{
    string id;
    int release_time;
    int exec_time;
    int period;
    int deadline_relative;
    int absolute_deadline;
    float priority=0.0;
    bool isActive=false;
    int remaining_exec = 0;
    Task() = default;
    Task(string id_, int r, int e, int p, int d_rel)
        : id(id_), release_time(r), exec_time(e),
          period(p), deadline_relative(d_rel)
    {
        absolute_deadline = release_time + deadline_relative;
        remaining_exec    = exec_time;
    }
    Task(string id,int r,int e):id(id),release_time(r),exec_time(e){}

};
void rateMonotonic(vector<Task> &tasks){
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }
     for (auto &t : tasks) {
        t.priority       = 1.0f / t.period;
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
    }
    
    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }

        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;

        sortByExplicitPriority(currentTasks);
        int runningTaskIndex=0;

        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else break;
        }
        if(runningTaskIndex==currentTasks.size()){
            cout<<time<<" is IDLE\n";
            continue;
        }
        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;

            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

void deadlineMonotonic(vector<Task> &tasks){
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }
     for (auto &t : tasks) {
        t.priority       = 1.0f / t.deadline_relative; //only difference here
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
    }
    
    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }

        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;

        sortByExplicitPriority(currentTasks);
        int runningTaskIndex=0;

        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else break;
        }
        if(runningTaskIndex==currentTasks.size()){
            cout<<time<<" is IDLE\n";
            continue;
        }
        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;

            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

void earliestDeadlineFirst(vector<Task> &tasks){
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }
     for (auto &t : tasks) {
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
    }
    
    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }

        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;

        sortByEarliestDeadline(currentTasks);
        int runningTaskIndex=0;

        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else break;
        }
        if(runningTaskIndex==currentTasks.size()){
            cout<<time<<" is IDLE\n";
            continue;
        }
        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;

            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

void leastLaxityFirst(vector<Task> &tasks){
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }
     for (auto &t : tasks) {
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
        t.priority=0.0;
    }
    
    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }

        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;
        
        for (auto& task : currentTasks) task.priority = task.absolute_deadline - task.remaining_exec - time;


        sortByLeastLaxity(currentTasks);
        int runningTaskIndex=0;

        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else break;
        }
        if(runningTaskIndex==currentTasks.size()){
            cout<<time<<" is IDLE\n";
            continue;
        }
        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;

            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

void backgroundServer(vector<Task> &tasks,vector<Task> &aperiodicTasks,int choose){
    //1 for rm 2 for dm 3 for edf 4 for llf
    if(choose!=1 && choose!=2 && choose!=3 && choose!=4){
        cout<<"Unknown scheduling for periodics";
        return;
    }
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }
     for (auto &t : tasks) {
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
        if(choose==1)t.priority=1.0f/t.period;
        else if(choose==2)t.priority=1.0f/t.deadline_relative; // as they are static priorities
    }
    for(auto &at: aperiodicTasks){
        at.absolute_deadline=INT_MAX;
        at.period=INT_MAX;
        at.deadline_relative=INT_MAX;
        at.isActive=false;
        at.remaining_exec=at.exec_time;
    }

    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    vector<Task> currentAperiodics=aperiodicTasks;

    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }

        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;

        if(choose==1 || choose==2) sortByExplicitPriority(currentTasks);
        if(choose==3)sortByEarliestDeadline(currentTasks); // 3 and 4 are dynamic priorities
        if(choose==4){
            for (auto& task : currentTasks) task.priority = task.absolute_deadline - task.remaining_exec - time;
            sortByLeastLaxity(currentTasks);
        }

        int runningTaskIndex=0;
        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else break;
        }
        //handling aperiodic tasks here
        if(runningTaskIndex==currentTasks.size()){
            cout<<"System can run aperiodic task at time:"<< time <<".\n";
            if(currentAperiodics.empty()){
                cout<<"But there is no aperiodic Task. It is IDLE time\n";
                continue;
            }
            sortTasksByReleaseTime(currentAperiodics); // first come first serve(the one first came at the first index of array)
            if(currentAperiodics[0].release_time<=time){
                currentAperiodics[0].remaining_exec--;
                cout<<"Aperiodic Task: "<<currentAperiodics[0].id<<" is running.\n";
                if(currentAperiodics[0].remaining_exec==0){
                    currentAperiodics.erase(currentAperiodics.begin());
                }
            }else{
                cout<<"But no aperiodic task released yet. It is IDLE time\n";
            }
            continue;
        }

        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;

            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

struct Server{
    string ID;
    int period;
    int budget;
    Server(string id,int p,int b):ID(id),period(p),budget(b){}
};
void pollerServer(vector<Task> &tasks,vector<Task> &aperiodicTasks,int choose,Server& poller){
    //1 for rm 2 for dm 3 for edf 4 for llf
    if(choose!=1 && choose!=2 && choose!=3 && choose!=4){
        cout<<"Unknown scheduling for periodics";
        return;
    }
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }

    Task serverTask(poller.ID,0,poller.budget,poller.period,poller.period);
    serverTask.isActive=true;
    tasks.push_back(serverTask);

    for (auto &t : tasks) {
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
        if(choose==1)t.priority=1.0f/t.period;
        else if(choose==2)t.priority=1.0f/t.deadline_relative; // as they are static priorities
    }

    sortTasksByReleaseTime(aperiodicTasks);
    for(auto &at: aperiodicTasks){
        at.absolute_deadline=INT_MAX;
        at.period=INT_MAX;
        at.deadline_relative=INT_MAX;
        at.isActive=false;
        at.remaining_exec=at.exec_time;
    }

    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    vector<Task> currentAperiodics=aperiodicTasks;

    int usedBudget=0;
    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.id==poller.ID)continue;
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }
        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;

        if(choose==1 || choose==2) sortByExplicitPriority(currentTasks);
        if(choose==3)sortByEarliestDeadline(currentTasks); // 3 and 4 are dynamic priorities
        if(choose==4){
            for (auto& task : currentTasks) task.priority = task.absolute_deadline - task.remaining_exec - time;
            sortByLeastLaxity(currentTasks);
        }

        if(time%poller.period==0){
            usedBudget=0;
            for(auto & at:currentAperiodics){
                if(at.release_time<=time){
                    if(at.remaining_exec<=poller.budget-usedBudget){
                        usedBudget+=at.remaining_exec;
                    }else{
                        usedBudget=poller.budget;
                    }   
                }
                if(usedBudget==poller.budget)break;
            }
            for(auto& t:currentTasks){
                if(t.id==poller.ID)t.remaining_exec=usedBudget;
            }
        }

        int runningTaskIndex=0;
        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else{
                if(task.id==poller.ID && task.remaining_exec==0){
                    runningTaskIndex++;
                }else break;
            } 
        }
        
        if(runningTaskIndex==currentTasks.size()){
            cout<<"System is Idle at "<< time <<".\n";
            continue;
        }

        Task& running=currentTasks[runningTaskIndex];
        if(running.id==poller.ID){
            cout << "At time: " << time << " Server " << running.id 
            << " serving " << currentAperiodics[0].id 
            << " (budget: " << running.remaining_exec << ")\n";

            running.remaining_exec--;
            currentAperiodics[0].remaining_exec--; //if we have no ready aperiodic task server.remaining_exec will be 0 so runningtask index will never be here 
            if(currentAperiodics[0].remaining_exec==0){
                currentAperiodics.erase(currentAperiodics.begin());
            }
            continue;
        }

        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;
            
            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

void deferrableServer(vector<Task> &tasks,vector<Task> &aperiodicTasks,int choose,Server& df){
    //1 for rm 2 for dm 3 for edf 4 for llf
    if(choose!=1 && choose!=2 && choose!=3 && choose!=4){
        cout<<"Unknown scheduling for periodics";
        return;
    }
    if(!checkFeasibility(tasks)){
        cout<<"This task set is not schedulable";
        return;
    }

    Task serverTask(df.ID,0,df.budget,df.period,df.period);
    serverTask.isActive=true;
    tasks.push_back(serverTask);

    for (auto &t : tasks) {
        t.absolute_deadline = t.release_time + t.deadline_relative;
        t.remaining_exec = t.exec_time;
        t.isActive       = false;
        if(choose==1)t.priority=1.0f/t.period;
        else if(choose==2)t.priority=1.0f/t.deadline_relative; // as they are static priorities
    }

    sortTasksByReleaseTime(aperiodicTasks);
    for(auto &at: aperiodicTasks){
        at.absolute_deadline=INT_MAX;
        at.period=INT_MAX;
        at.deadline_relative=INT_MAX;
        at.isActive=false;
        at.remaining_exec=at.exec_time;
    }

    int hyperperiod=calculate_hyperperiod(tasks);
    int firstSimoultaneous=findFirstSimultaneousRelease(tasks);
    vector<Task> currentTasks=tasks;
    vector<Task> currentAperiodics=aperiodicTasks;

    for(int time=0; time<hyperperiod+firstSimoultaneous+1; time++){
        for(auto &ct:currentTasks){
            if(ct.id==df.ID)continue;
            if(ct.absolute_deadline<=time){
                cout<<"Deadline missed for "<< ct.id<<" at " <<time <<". Algorithm is failed.\n";
                return;
            }
        }
        for (auto &task : currentTasks) if(task.release_time<=time)task.isActive=true;
        
        if(time%df.period==0){
            for(auto& t:currentTasks){
                if(t.id==df.ID){
                    t.remaining_exec=df.budget;
                    t.release_time=time;
                    t.absolute_deadline=time+df.period;
                    break;
                }
            }
        }

        if(choose==1 || choose==2) sortByExplicitPriority(currentTasks);
        if(choose==3)sortByEarliestDeadline(currentTasks); // 3 and 4 are dynamic priorities
        if(choose==4){
            for (auto& task : currentTasks) task.priority = task.absolute_deadline - task.remaining_exec - time;
            sortByLeastLaxity(currentTasks);
        }


        bool isThereAperiodicTask=false;
        for(auto& task:currentAperiodics){
            if(task.release_time<=time)isThereAperiodicTask=true;
        }

        int runningTaskIndex=0;
        for (auto &task : currentTasks){
            if(task.isActive==false)runningTaskIndex++;
            else{
                if(task.id==df.ID && (task.remaining_exec==0 || !isThereAperiodicTask)){
                    runningTaskIndex++;
                }else break;
            } 
        }
        
        if(runningTaskIndex==currentTasks.size()){
            cout<<"System is Idle at "<< time <<".\n";
            continue;
        }

        Task& running=currentTasks[runningTaskIndex];
        if(running.id==df.ID){
            cout << "At time: " << time << " Server " << running.id 
            << " serving " << currentAperiodics[0].id 
            << " (budget: " << running.remaining_exec << ")\n";

            running.remaining_exec--;
            currentAperiodics[0].remaining_exec--; //if we have no ready aperiodic task server.remaining_exec will be 0 so runningtask index will never be here 
            if(currentAperiodics[0].remaining_exec==0){
                currentAperiodics.erase(currentAperiodics.begin());
            }
            continue;
        }

        cout<<"At time: "<< time<<" Task" << currentTasks[runningTaskIndex].id<<" is running\n";

        currentTasks[runningTaskIndex].remaining_exec--;
        if(currentTasks[runningTaskIndex].remaining_exec==0){
            Task oldTask=currentTasks[runningTaskIndex];
            Task newTask=oldTask;
            
            newTask.isActive=false;
            newTask.release_time=oldTask.release_time+oldTask.period;
            newTask.absolute_deadline=newTask.release_time+newTask.deadline_relative;
            newTask.remaining_exec=newTask.exec_time;

            currentTasks.erase(currentTasks.begin()+runningTaskIndex);
            currentTasks.push_back(newTask);
        }
    }
    cout<<"Successfully scheduled until hyperperiod.\n";
}

int main(int argc, char* argv[]){
    if (argc < 3) {
        cerr << "Wrong Argument Count!" << endl;
        return 1;
    }
    string filename = argv[1];
    string algoStr = argv[2];
    int algoCode = getAlgorithmCode(algoStr);

    if (algoCode == 0) {
        cerr << "Wrong scheduling algorithm (Only RM, DM, EDF, LLF)." << endl;
        return 1;
    }

    vector<Task> periodicTasks;
    vector<Task> aperiodicTasks;
    readInputFile(filename, periodicTasks, aperiodicTasks);

    if (argc == 3) {
        cout << "Running Periodic Scheduling: " << algoStr << endl;
        if (algoCode == 1) rateMonotonic(periodicTasks);
        else if (algoCode == 2) deadlineMonotonic(periodicTasks);
        else if (algoCode == 3) earliestDeadlineFirst(periodicTasks);
        else if (algoCode == 4) leastLaxityFirst(periodicTasks);
    }
    else {
        string serverType = getServerType(argv[3]);

        if (serverType == "BG") {
            backgroundServer(periodicTasks, aperiodicTasks, algoCode);
        }
        else if (serverType == "POLLER" || serverType == "DS") {            
            if (argc != 6) {
                cerr << "Wrong format: " << serverType << " Need budget and Period" << endl;
                return 1;
            }
            int budget = stoi(argv[4]);
            int period = stoi(argv[5]);
            Server sObj("ServerTask", period, budget);

            if (serverType == "POLLER") {
                pollerServer(periodicTasks, aperiodicTasks, algoCode, sObj);
            } else {
                deferrableServer(periodicTasks, aperiodicTasks, algoCode, sObj);
            }
        }
        else {
            cerr << "Wrong server type.(Only BG,DS,POLLING)" << argv[3] << endl;
            return 1;
        }
    }
    return 0;
}

void sortTasksByReleaseTime(vector<Task>& tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.release_time < b.release_time;
    });
}

void sortByLeastLaxity(vector<Task>& tasks) {
    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        if (!a.isActive && b.isActive) return false;
        if (a.isActive && !b.isActive) return true;
        if (!a.isActive && !b.isActive) return false;
        
        return a.priority < b.priority;
    });
}

void sortByEarliestDeadline(vector<Task>& tasks) {
    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        if (!a.isActive && b.isActive) return false;
        if (a.isActive && !b.isActive) return true;
        if (!a.isActive && !b.isActive) return false;
        return a.absolute_deadline < b.absolute_deadline;
    });
}

void sortByExplicitPriority(vector<Task>& tasks) {
    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.priority > b.priority; 
    });
}
bool checkFeasibility(vector<Task>& tasks){
    double u = 0.0;
    for(const auto& task:tasks){
        u+=(double)task.exec_time / task.period;
    }
    if (u>1)return false;
    return true;
}
int gcd(int a, int b) {
    while (b) {
        a %= b;
        swap(a, b);
    }
    return a;
}

int lcm(int a, int b) {
    if (a == 0 || b == 0) return 0;
    return (a * b) / gcd(a, b);
}

int calculate_hyperperiod(const std::vector<Task>& tasks) {
    int hyperperiod = 1;
    for (const auto& task : tasks) {
        hyperperiod = lcm(hyperperiod, task.period);
    }
    return hyperperiod;
}

int findFirstSimultaneousRelease(const std::vector<Task>& tasks) {
    if (tasks.empty()) return -1;

    long long current_time = tasks[0].release_time;
    long long current_step = tasks[0].period;

    for (size_t i = 1; i < tasks.size(); ++i) {
        int r_next = tasks[i].release_time;
        int p_next = tasks[i].period;

        if ((r_next - current_time) % gcd(current_step, p_next) != 0) {
            std::cerr << "Warning: Task " << tasks[0].id << " Task " << tasks[i].id 
                      << "never release at the same time. So will be simulated only until hyperperiod" << std::endl;
            return 0; 
        }
        while (true) {
            long long relative_time = current_time - r_next;
            
            if (relative_time >= 0 && relative_time % p_next == 0) {
                break;
            }
            
            current_time += current_step;
        }
        current_step = lcm(current_step, p_next);
    }

    return current_time;
}


string getServerType(string s) {
    transform(s.begin(), s.end(), s.begin(), ::toupper);

    if (s == "BACKGROUND" || s == "BG") return "BG";
    if (s == "POLLER" || s == "POLLING") return "POLLER";
    if (s == "DEFERRABLE" || s == "DS") return "DS";
    return "";
}

int getAlgorithmCode(string algo) {
    transform(algo.begin(), algo.end(), algo.begin(), ::toupper); 
    
    if (algo == "RM") return 1;
    if (algo == "DM") return 2;
    if (algo == "EDF") return 3;
    if (algo == "LLF") return 4;
    
    return 0; 
}

void readInputFile(const string& filename, vector<Task>& periodicTasks, vector<Task>& aperiodicTasks) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: " << filename << " file could not be opened!" << endl;
        return;
    }

    string line;
    int p_counter = 1; 
    int a_counter = 1; 

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string type;
        ss >> type; 

        vector<int> values;
        int val;
        while (ss >> val) {
            values.push_back(val);
        }

        if (type == "P") {
            string id = "T" + to_string(p_counter++);
            int r, e, p, d;

            if (values.size() == 4) {
                r = values[0];
                e = values[1];
                p = values[2];
                d = values[3];
            }
            else if (values.size() == 3) {
                r = values[0];
                e = values[1];
                p = values[2];
                d = p; 
            }
            else if (values.size() == 2) {
                r = 0;
                e = values[0];
                p = values[1];
                d = p;
            } else {
                cerr << "wrong format: " << line << endl;
                continue;
            }
            
            periodicTasks.emplace_back(id, r, e, p, d);
        }
        
        else if (type == "D") {
            if (values.size() == 3) {
                string id = "T" + to_string(p_counter++);
                int r = 0;
                int e = values[0];
                int p = values[1];
                int d = values[2];

                periodicTasks.emplace_back(id, r, e, p, d);
            } else {
                cerr << "wrong format: " << line << endl;
            }
        }

        else if (type == "A") {
            if (values.size() == 2) {
                string id = "A" + to_string(a_counter++);
                int r = values[0];
                int e = values[1];
                
                aperiodicTasks.emplace_back(id, r, e, INT_MAX, INT_MAX);
            } else {
                cerr << "wrong format: " << line << endl;
            }
        }
    }

    file.close();
    cout << filename << " file read succesfully. ";
    cout << "Periodic Task Count: " << periodicTasks.size();
    cout << "Aperiodic Task Count: " << aperiodicTasks.size() << endl;
}