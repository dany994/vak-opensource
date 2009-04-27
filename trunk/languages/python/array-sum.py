#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Задача: в заданном массиве целых чисел найти отрезок с максимальной суммой.
# Ссылка: http://www.cprogramming.com/challenges/array_sum.html
# Массив разрешается просматривать всего один раз.
# Дополнительную память использовать нельзя.
#
array = [3, -2, 1, 4, 5, 2, -7, 3]

fsum = 0
bsum = 0
last = -1
first = -1
n = len (array)
for i in range (0, n):
	fsum += array[i]
	if array[i] > 0:
		if last < 0 or fsum > fmax:
			fmax = fsum
			last = i
	bsum += array[n-1-i]
	if array[n-1-i] > 0:
		if first < 0 or bsum > bmax:
			bmax = bsum
			first = n-1-i

print "Array:", array
if first >= 0:
	print "Solution:", array [first : last+1]
else:
	print "Solution: empty subarray"
