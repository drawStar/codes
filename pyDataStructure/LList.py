#coding=utf-8
#无头结点的单链表，目前不知道带头结点的怎么实现
#def append
class LNode:
    def __init__(self,elem,next_=None):
        self.elem=elem
        self.next_=next_
class LList:
    def __init__(self):
        self._head=None
    def append(self,elem):
        #尾插法
        if self._head is None:
           self._head=LNode(elem)#这样可以
           #p=LNode(elem)#这样就不行
        else:
            p = self._head
            while p.next_ is not None:
                p=p.next_
            p.next_=LNode(elem,None)
    def isEmpty(self):
        return self._head is None
    def prepend(self,elem):
        #头插法
        self._head=LNode(elem,self._head)
    def pop(self):
        #删除表头结点并返回数据
        if self._head is None:
            raise Exception("抛出一个异常")
        temp=self._head.elem
        self._head=self._head.next_
        return temp
    def poplast(self):
        p=self._head
        if self._head.next_ is None:
            x=self._head.elem
            self._head=None
            return x
        temp=p
        while p.next_ is not None:
            temp=p
            p=p.next_
            x=p.elem
        temp.next_=None
        return x
    def sort1(self):
        #插入排序，通过交换元素方式实现
        crt=self._head.next_ #从第二个结点处理
        while crt is not None:
            p=self._head
            while p is not crt and p.elem<=crt.elem:#跳过比crt指向的数据小的数据
                p=p.next_
            while p is not crt:
                p.elem, crt.elem = crt.elem, p.elem
                p=p.next_
            crt=crt.next_




    def printAll(self):
        p=self._head
        while p is not None:
            print p.elem
            p=p.next_


mylist=LList()
mylist.append(22)
mylist.append(20)
mylist.append(7)
mylist.append(12)
mylist.sort1()
mylist.printAll()




