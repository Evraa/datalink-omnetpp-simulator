from faker import Faker


fake = Faker('en_US')
f = open("msgs.txt","w")
for i in range(1000):   
    line = fake.text(max_nb_chars=100)
    f.write(line)
    f.write("\n")

