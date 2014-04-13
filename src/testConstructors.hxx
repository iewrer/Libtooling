class Flower
{
int m_color;
int m_scent;

public:

Flower(int color, int scent);
int getColor();
int getScent();
    void getnothing();
};

class Rose: public Flower
{
int m_age;

public:
Rose(int color, int scent, int age);
int getAge();
};
