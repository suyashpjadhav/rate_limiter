# Rate Limiter Implementation Notes

## How Thread Safety is Guaranteed
To ensure thread-safe execution when multiple threads call `allowRequest()` simultaneously, I utilized a **Sliding Window Log** algorithm protected by a **Mutex (Mutual Exclusion)**. 

*   **Atomicity Through Mutual Exclusion:** Inside `allowRequest()`, the very first operation is acquiring a `mutex` via a `lock_guard`. This creates a critical section, ensuring that only one thread can inspect, evict, or append timestamps to the queue at any given moment.
*   **RAII Compliance:** By using `lock_guard`, the mutex is tied to the scope of the function. It automatically releases the lock when the function finishes (whether it returns `true` or `false`) or if an exception occurs, ensuring that the lock is not accidentally left locked.
*   **Monotonic Clock:** I used `steady_clock` instead of the system wall-clock. This prevents thread-safety and logic issues that could occur if the operating system's time is adjusted mid-execution.

## Algorithm and Complexity

- **Algorithm:** Sliding Window Log.
- **Time Complexity:** O(k) in the worst case per call, where k is the number of expired timestamps removed. Each timestamp is inserted once and removed once, so cleanup takes O(1) time on average per timestamp over multiple calls.
- **Space Complexity:** O(N), since at most N successful request timestamps are stored.
- **Synchronization:** Mutex-based mutual exclusion using `lock_guard` to ensure thread-safe access to shared data.

## Uncertainties and Assumptions
While the implementation satisfies the core requirements, there were a few architectural unknowns I had to make assumptions about:

1.  **Deployment Environment:** I assumed this system runs on a **single instance/node**. If this code is deployed across a distributed environment (e.g., multiple microservice instances), a local in-memory lock will not prevent the global API limit from being exceeded.
2.  **Memory Constraints (The Value of N):** I assumed `N` is a reasonably small number. Because the Sliding Window Log stores a timestamp for *every* successful request, an excessively large `N` (e.g., millions of requests per second) would consume significant memory. If memory is strictly constrained and `N` is huge, switching to a memory-efficient algorithm like the Token Bucket would be necessary.
3.  **Client Behavior:** The method simply returns `true` or `false` (non-blocking). I assumed the calling threads are responsible for handling their own retry logic (e.g., exponential backoff) rather than the rate limiter blocking the thread until capacity frees up.