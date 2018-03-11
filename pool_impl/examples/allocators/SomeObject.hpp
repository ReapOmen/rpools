#ifndef __SOME_OBJECT_H__
#define __SOME_OBJECT_H__

class SomeObject {
public:
    void setX(int t_x) { m_x = t_x; }
    void setY(int t_y) { m_y = t_y; }
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int prod() const { return m_x * m_y; }
private:
    int m_x, m_y;
};

#endif
