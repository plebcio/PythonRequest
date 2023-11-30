import json, sys

# add 3 values together
def addThree(a, b, c):
    return a + b + c

def main():
    s = sys.stdin.readline()
    data = json.loads(s)
    ret = {}
    try:
        ret["res"] = addThree(**data)
        ret["err"] = False
    except Exception as e:
        ret["err"] = True
        ret["what"] = str(e)

    print(json.dumps(ret), end="\n\0")

main()