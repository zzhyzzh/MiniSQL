import random
num = 1500;
letters = list('abcdefghijklmnopqrstuvwxyz')

file = open("huge.sql","w")

file.write("create table test (attr1 int, attr2 float, attr3 char(10), primary key(attr1));\n")
for x in range(0,num):
    file.write("insert into test values(" + str(x) + ',' + str(random.randint(1,1000)) + '.' + str(random.randint(1,1000))+ ",'" + letters[random.randint(0, 25)] +"');\n" )
