#coding=utf-8
#字符串匹配
#http://www.ruanyifeng.com/blog/2013/05/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm.html
def naiveMatching(t,p):
    #朴素的串匹配算法,t是主串，p是模式串
    i,j=0,0
    m,n=len(t),len(p)
    while i<m and j<n:
        if (t[i] == p[j]):
            i,j=i+1,j+1
        else:
            # print i,j,k
            i=i-j+1
            j=0
    if j==n:
        return i-j
    return -1

# def KMP

t = "abbabaaba"
p = "aaba"

s=naiveMatching(t,p)
print s

