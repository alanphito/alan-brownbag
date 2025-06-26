#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <numeric>
#include <limits>

std::mutex cout_mutex;// for print safely

struct Task {//represents unit of work
  int id;
  std::chrono::high_resolution_clock::time_point fetch_time;
};

void fetch_task(Task& task, int fetch_time_ms) { //simulates time taken to fetch a task
  std::this_thread::sleep_for(std::chrono::milliseconds(fetch_time_ms));
  task.fetch_time = std::chrono::high_resolution_clock::now();
}

void process_task(const Task& task, std::vector<long long>& latencies, int process_time_ms) {//simulates task processing, measures latency
  std::this_thread::sleep_for(std::chrono::milliseconds(process_time_ms));
  auto end = std::chrono::high_resolution_clock::now();
  long long latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - task.fetch_time).count();
  latencies[task.id] = latency;

  std::lock_guard<std::mutex> lock(cout_mutex);
  std::cout << "Task " << task.id << " latency: " << latency << " ms\n";
}

//simulation based on system spectrum as parameter
void simulate_mode(const std::string& label, int task_count, int fetch_time_ms, int process_time_ms, int num_threads) {
  std::cout << "\n=== " << label << " ===\n";
  std::vector<long long> latencies(task_count);
  std::queue<Task> task_queue;
  std::mutex queue_mutex;
  std::condition_variable cv;
  std::atomic<int> completed_tasks{0};
  bool done = false;

  auto worker = [&]() {//proccesses tasks once arrives to them
    while (true) {
      Task task;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [&]() { return !task_queue.empty() || done; });
        if (task_queue.empty() && done) break;
        task = task_queue.front();
        task_queue.pop();
      }
      process_task(task, latencies, process_time_ms);
      completed_tasks++;
    }
  };

  auto start = std::chrono::high_resolution_clock::now();

  //launches workers
  std::vector<std::thread> workers;
  for (int i = 0; i < num_threads; ++i)
    workers.emplace_back(worker);

  //simulates fetching and adding tasks to queue
  for (int i = 0; i < task_count; ++i) {
    Task task{.id = i};
    fetch_task(task, fetch_time_ms);
    {
      std::lock_guard<std::mutex> lock(queue_mutex);
      task_queue.push(task);
    }
    cv.notify_one();
  }
  
  //notifies workers that no tasks will be fetched anymore
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    done = true;
  }
  cv.notify_all();

  //waits for all threads to be finished
  for (auto& w : workers)
    w.join();

  auto end = std::chrono::high_resolution_clock::now();
  auto total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  double average_latency = std::accumulate(latencies.begin(), latencies.end(), 0LL) / static_cast<double>(task_count);

  std::cout << "Total time: " << total << " ms\n";
  std::cout << "Average latency: " << average_latency << " ms\n";
}


void pause() {//pause before each simulation
  std::cout << "\nPress Enter to continue...\n";
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
  const int task_count = 10; //number of tasks to simulate

  simulate_mode("High Latency, High Throughput", task_count, 100, 200, std::thread::hardware_concurrency());
  pause();
  
  simulate_mode("Low Latency, Low Throughput", task_count, 10, 20, 1);
  pause();
  
  simulate_mode("Low Latency, High Throughput", task_count, 10, 20, std::thread::hardware_concurrency());
  pause();
  
  simulate_mode("High Latency, Low Throughput", task_count, 100, 200, 1);
  
  return 0;
}

