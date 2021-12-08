# To create a https nodejs server
With self signed certificate

## Look here:
https://stackoverflow.com/questions/11744975/enabling-https-on-express-js

```bash
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout ./selfsigned.key -out selfsigned.crt
```



