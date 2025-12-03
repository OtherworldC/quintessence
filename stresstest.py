import asyncio
import aiohttp
import time

# List of endpoints to stress test
ENDPOINTS = [
    "http://localhost:8080/",
    "http://localhost:8080/index.html",
    "http://localhost:8080/style/style.css",
    "http://localhost:8080/js/index.js",
    "http://localhost:8080/404.html",
    "http://localhost:8080/mnij.mp4",
    "http://localhost:8080/test.mp3",
]

CONCURRENT_REQUESTS = 20
TEST_DURATION = 10  # seconds

async def fetch(session, url):
    start_time = time.monotonic()
    try:
        async with session.get(url) as response:
            await response.text()  # Read response to ensure the request completes
            elapsed_time = time.monotonic() - start_time
            return url, response.status, elapsed_time
    except Exception as e:
        elapsed_time = time.monotonic() - start_time
        return url, None, elapsed_time

async def worker(session, results, stop_event):
    while not stop_event.is_set():
        for endpoint in ENDPOINTS:
            if stop_event.is_set():
                break
            result = await fetch(session, endpoint)
            results.append(result)

async def stress_test():
    results = []
    stop_event = asyncio.Event()

    async with aiohttp.ClientSession() as session:
        # Start workers
        tasks = [
            asyncio.create_task(worker(session, results, stop_event))
            for _ in range(CONCURRENT_REQUESTS)
        ]

        # Run the test for the specified duration
        await asyncio.sleep(TEST_DURATION)
        stop_event.set()

        # Wait for all workers to finish
        await asyncio.gather(*tasks)

    # Print results
    print("Stress Test Results:")
    for url, status, elapsed_time in results:
        print(f"URL: {url}, Status: {status}, Response Time: {elapsed_time:.4f} seconds")

if __name__ == "__main__":
    asyncio.run(stress_test())