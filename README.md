# term_paper
**Build and compile**

**Server:**
```
g++ -o termpaper main.cpp task_queue.cpp inverted_index.cpp
```
```
./termpaper
```

**Result:**

![зображення](https://github.com/user-attachments/assets/7a1b1903-5d00-4794-870f-32d034b50763)

**Client cpp:**
```
g++ -o client_cpp client.cpp
```
```
./client_cpp
```

**Result:**

![зображення](https://github.com/user-attachments/assets/9254220d-b641-4143-896f-d8c1405478e7)

**Manual request to the server**
**Add file:**
```
telnet localhost 12345
```
```
ADD text1 Manual request
```

**Result:**

![зображення](https://github.com/user-attachments/assets/d043ef36-da52-4ccb-8443-9ade51dd9de9)

**Search file:**
```
telnet localhost 12345
```
```
SEARCH request
```

**Result:**

![зображення](https://github.com/user-attachments/assets/0db7fcd0-1d97-40b7-ba59-0a0ea8049b6e)


**Delete file:**
```
telnet localhost 12345
```
```
DELETE text1
```

**Result:**

![зображення](https://github.com/user-attachments/assets/bb87d6ba-8688-4539-9008-da74191055bf)


