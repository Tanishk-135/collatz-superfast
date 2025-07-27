a=int(input("Enter >>> "))

count=0

while True:
    if a == 1:
        print("Number of steps:", count)
        break
    elif a % 2 == 0:
        a = a // 2
    else:
        a = 3 * a + 1
    count += 1