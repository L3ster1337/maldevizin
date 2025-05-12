import sys

def xor(data, key):
    return bytes([b ^ key[i % len(key)] for i, b in enumerate(data)])

XOR_KEY = b'secreta123' # here you can change for any key you want

with open(sys.argv[1], 'rb') as f:
    raw = f.read()

split = raw.split(b'\r\n\r\n', 1) # function that separates headers
if len(split) != 2:
    print("[-] Body not found for raw data")
    sys.exit(1)

body = split[1]  # allowing just what commes after the headers

decrypted = xor(body, XOR_KEY)

with open(sys.argv[2], 'wb') as f:
    f.write(decrypted)

print(f"[+] File remounted as: {sys.argv[2]}")
