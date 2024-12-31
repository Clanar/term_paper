# term_paper
**Build and compile**

**Server:**
```
g++ -o termpaper  main.cpp inverted_index.cpp
```
```
./termpaper
```

**Result:**

![зображення](https://github.com/user-attachments/assets/9ae8145f-d5c4-4ba1-b8f6-caaec5233ff3)

**Client cpp:**
```
g++ -o client_cpp client.cpp
```
```
./client_cpp
```

**Result:**

![зображення](https://github.com/user-attachments/assets/9c3f5862-b1cf-4705-b771-f2b3cabc3e0f)

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


