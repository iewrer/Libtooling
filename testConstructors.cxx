#include "testConstructors.hxx"
Flower flower(3,4);
Rose rose(2,3,4);

Flower::Flower(int color, int scent){
m_color = color;
m_scent = scent;
    getScent();
}

int Flower::getColor(){
return m_color;
}

int Flower::getScent(){
    getColor();
return m_scent;
}

Rose::Rose(int color, int scent, int age)
          :Flower(color, scent){
m_age = age;
}

int Rose::getAge(){
return m_age;
}

int main(){

//static Flower flower2(1,2);

}
