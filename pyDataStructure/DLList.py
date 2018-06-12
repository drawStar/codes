#coding=utf-8
#双链表类,带有prev和next
from LNode import LNode
from LList1 import LList1
class DLNode(LNode):
    def __init__(self,elem,prev_=None,next_=None):
        LNode.__init__(self,elem,next_)
        self.prev_=prev_

class DLList(LList1):
    def __init__(self):
        LList1.__init__(self)

    def append(self,elem):
        if self._head is None:
            self._head=DLNode(elem)
            self._rear=self._head
        else:
            p=DLNode(elem)
            p.prev_=self._rear
            self._rear.next_=p
            self._rear=p

    def prepend(self,elem):
        if self._head is None:
            self._head=DLNode(elem)
            self._rear=self._head
        else:
            p=DLNode(elem)
            p.next_=self._head
            self._head.prev_=p
            self._head=p
    def pop(self):
        if self._head is None:
            raise Exception("链表为空")
        else:
            p=self._head.next_
            temp=self._head.elem
            p.prev_=None
            self._head=self._head.next_
            return temp
    def poplast(self):
        if self._head is None:
            raise Exception("链表为空")
        else:
            temp=self._rear.elem
            p=self._rear.prev_
            p.next_=None
            self._rear=p
            return  temp
    def myReverse(self):
        #通过交换元素方式实现反转
        a=self._head
        b=self._rear
        while a != b and a.next_ is not None and b.prev_ is not None:
            a.elem,b.elem=b.elem,a.elem
            a=a.next_
            b=b.prev_



def sp(a):
    for i in range(1, len(a)):
        x = a[i]
        j = i
        while j > 0 and a[j - 1] > x:
            a[j] = a[j - 1]
            j -= 1
        a[j] = x



x=[1,3,4,5,6,7,2,4]
print x
sp(x)
print x