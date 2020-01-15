#include "jobscheduler.h"

using namespace std;

JobScheduler::JobScheduler() {
    activeJobs = 0;
    stop = false;
    q = new queue();
}

bool JobScheduler::Init(int numThreads) {
    this->numThreads = numThreads;
    threads = new thread[numThreads];
    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread(&JobScheduler::runJobs,this);
    }
    return true;
}

void JobScheduler::runJobs() {
    while (true) {
        Job *job;
        while(true) {
            unique_lock<mutex> lk(queuMu);
            while (q->empty() && !stop)
                cv.wait(lk);
            if (stop)
                return;
            job = q->pop();
            lk.unlock();
            if(job) break;
        }
        job->Run();
        removeActiveJobs();
    }
}

void JobScheduler::Schedule(Job *job) {
    addActiveJobs();
    unique_lock<mutex> lk(queuMu);
    q->push(job);
    lk.unlock();
    cv.notify_one();
}

void JobScheduler::addActiveJobs() {
    unique_lock<mutex> lk(activeJobsMu);
    activeJobs++;
    lk.unlock();
}

void JobScheduler::removeActiveJobs() {
    unique_lock<mutex> lk(activeJobsMu);
    activeJobs--;
    if (activeJobs <= 0) {
        activeJobsCv.notify_all();
    }
    lk.unlock();
}

void JobScheduler::Barrier() {
    unique_lock<mutex> lk(activeJobsMu);
    while(activeJobs > 0) //could have an if statement instead of while
        activeJobsCv.wait(lk);
    lk.unlock();
    //Destroy();
}

void JobScheduler::Stop() {
    unique_lock<mutex> lk(queuMu);
    stop = true;
    lk.unlock();
    cv.notify_all();
    Destroy();
}

bool JobScheduler::Destroy() {
    for (int i =0; i < numThreads; i++) {
        threads[i].join();
    }
}