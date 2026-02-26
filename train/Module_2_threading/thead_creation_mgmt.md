# thread creation & mgmt
You're developing a data analytics platform that processes customer transaction data from multiple sources simultaneously. Each data source runs on a separate thread to maximize throughput.
🔍 Practice
Run the code below and observe the output. Think about:
How does std::lock_guard ensure thread safety when accessing sharedData?
Why is std::atomic used for processedCount instead of a regular integer?
Modify the code to create 5 threads, each processing 20 data items. Test your changes and observe:
How the total processed count matches the expected value (100)
Whether the output messages from different threads appear in order

✅ Success Checklist
All threads complete without data races or corruption
The atomic counter shows the correct total number of processed items
Shared vector contains data from all threads without missing entries

# 