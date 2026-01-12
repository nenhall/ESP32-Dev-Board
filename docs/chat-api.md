curl --location '192.168.0.103:10089/chat/completions/' \
--header 'X-Conversation-Id: bzsz_1768214958_843677' \
--header 'Content-Type: application/json' \
--header 'Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkZW50aWZpZXIiOiIxMzUzZTE2ODY1NjI1MTY4YzJjMWNiMjI0MjQ5YjdhYiIsImV4cCI6MTc2ODMwMTI1MCwiaWF0IjoxNzY4MjE0ODUwLCJpc3MiOiJmYXN0YXBpLWNoYXQtc2VydmljZSIsInR5cGUiOiJhY2Nlc3MifQ.9voLX6mJpkQNatnXNp831kDSkp8mfTmVM8q7CfVKc9I' \
--data '{
    "model": "glm",
    "store": true,
    "stream": true,
    "messages": [
        {
            "content": "重阳节",
            "role": "user",
            "name": "string"
        }
    ],
    "response_format": {
        "type": "text"
    }, 
    "user": "1353e16865625168c2c1cb224249b7ab",
    "reasoning": false,
    "thinking": false,
    "web_search": {
        "enable": true
    },
    "scene": "chat"
}'