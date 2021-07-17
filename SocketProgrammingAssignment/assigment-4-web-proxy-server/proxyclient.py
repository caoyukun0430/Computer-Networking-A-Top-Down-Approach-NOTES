import requests
# python way to send post request
pload = {'username':'yukun','password':'123'}
# pload = {'@number': 12524, '@type': 'issue', '@action': 'show'}
# pload =  {"user": "yukun", "year": "2009"}
# r = requests.post('https://httpbin.org/post',data = pload)
# r = requests.post('https://bugs.python.org',data = pload)
r = requests.post('http://postman-echo.com/post/',data = pload)
print(r.headers)
print(r.text)
# str1 = "Issue 12524"
# if str1 in r.text:
#     print("yes")