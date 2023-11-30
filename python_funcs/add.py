import json, sys
def add(a, b):
    return a+b
def main():
    s = sys.stdin.readline()
    data = json.loads(s)
    ret = {}
    try:
        ret['res'] = add(**data)
        ret['err'] = False
    except Exception as e:
         ret['err'] = True
         ret['what'] = str(e)
    print(json.dumps(ret), end='\n\0')
main()
