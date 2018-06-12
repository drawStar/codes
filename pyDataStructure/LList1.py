#coding=utf-8
#在LList的基础上增加尾结点域，提高尾端操作的效率

from LList import  LList
from LList import LNode
class LList1(LList):
    def __init__(self):
        LList.__init__(self) #先初始化父类LList的数据
        self._rear=None
    def prepend(self,elem):
        if self._head is None:
            self._head = LNode(elem)
            self._rear=self._head
        else:
            self._head=LNode(elem,self._head)
    def append(self,elem):
        if self._head is None:
            self._head=LNode(elem)
            self._rear=self._head
        else:
            self._rear.next_=LNode(elem)
            self._rear=self._rear.next_
    def poplast(self):
        if self._head is None:
            raise Exception("链表为空")
        else:
            if self._head.next_ is None:#表中只有一个元素
                temp=self._head.elem
                self._rear = None
                self._head = None
                return temp
            else:
                p=self._head
                while p.next_.next_ is not None:
                    p=p.next_
                temp=self._rear.elem
                self._rear=p
                self._rear.next_=None
                return temp
