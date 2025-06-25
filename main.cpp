#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

const int TASK_COUNT = 10;
const int FETCH_TIME_MS = 30;
const int PROCESS_TIME_MS = 70;

std::mutex coutMutex;

struct Task {
  int id;
  std::chrono::high_resolution_clock::time_point fetchTime;
};

void fetchTask(Task& task) {
  std::this_thread::sleep_for(std::chrono::milliseconds(FETCH_TIME_MS));
  task.fetchTime = std::chrono::high_resolution_clock::now();
}

void processTask(const Task& task, std::vector<long long>& latencies) {
  std::this_thread::sleep_for(std::chrono::milliseconds(PROCESS_TIME_MS));
  auto end = std::chrono::high_resolution_clock::now();

  long long latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - task.fetchTime).count();
  latencies[task.id] = latency;

  std::lock_guard<std::mutex> lock(coutMutex);
  std::cout << "Task " << task.id << " latency: " << latency << " ms\n";
}

void lowLatencyMode() {
  std::cout << "\n=== LOW LATENCY MODE ===\n";
  std::vector<long long> latencies(TASK_COUNT);

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < TASK_COUNT; ++i) {
    Task task{.id = i};
    fetchTask(task);       // fetching
    processTask(task, latencies); // processing
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "Total time (Low Latency): " << total << " ms\n";
}

void highThroughputMode() {
  std::cout << "\n=== HIGH THROUGHPUT MODE ===\n";
  std::vector<long long> latencies(TASK_COUNT);
  std::queue<Task> taskQueue;
  std::mutex queueMutex;
  std::condition_variable cv;
  std::atomic<int> completedTasks{0};
  bool doneFetching = false;

  auto worker = [&]() {//Worker thread
    while (true) {
      Task task;
      {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&]() { return !taskQueue.empty() || doneFetching; });
        if (taskQueue.empty() && doneFetching) break;
        task = taskQueue.front();
        taskQueue.pop();
      }
      processTask(task, latencies);
      completedTasks++;
    }
  };

  auto start = std::chrono::high_resolution_clock::now();

  //simulate concurrency
  int numThreads = std::thread::hardware_concurrency();
  std::vector<std::thread> workers;
  for (int i = 0; i < numThreads; ++i) {
    workers.emplace_back(worker);
  }

  //Fetch tasks in batch and enqueue
  for (int i = 0; i < TASK_COUNT; ++i) {
    Task task{.id = i};
    fetchTask(task);  // simulate network/db fetch
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      taskQueue.push(task);
    }
    cv.notify_one();
  }

  //Notify workers no more tasks will arrive
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    doneFetching = true;
  }
  cv.notify_all();

  //Waits for all workers to finish
  for (auto& w : workers) {
    w.join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "Total time (High Throughput): " << total << " ms\n";
}

int main() {
  lowLatencyMode();
  highThroughputMode();
  return 0;
}

