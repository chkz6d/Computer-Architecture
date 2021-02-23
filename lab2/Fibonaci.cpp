#include <iostream>
using namespace std;

int main()
{
	int prev, N, next =1, final =0;
	
	cout << "Enter N: ";
	cin >> N;

	for(int i = 0; i <= N; i++)
	{
		if(i <= 1)
		{
			cout << final << " ";
			final++;
		}
		else
		{
			final = next + prev;
			prev = next;
			next = final;
			cout << final << " ";
		}
	}
	
	return 0;
}


/* MIPS Assembly

[0x04000000]    addiu, $a0, $zero, 0 //set final to zero
[0x04000004]    addiu, $a1, $zero, 1 //set next to one
[0x04000008]    addiu, $a2, $zero, 0 //set prev to 0
[0x0400000C]    addiu, $a3, $zero, 0xA //set i upper bound to 10
[0x04000010]    addiu, $a4, $zero, 0x2 //set i lower bound is 2
[0x04000014]    addiu, $a5, $zero, 0 //i is equal to 0
[0x04000018]    addiu, $a5, $a5, 0x01 //i + 1
[0x0400001C]    beq, $a4. $a5, 0x0C //if branch taken jump to 0x04000028
[0x04000020]    addiu, $a0, $a0, 0x01 //final + 1
[0x04000024]    j 4194328
[0x04000028]    add, $a0, $a1, $a2//final = next + prev
[0x0400002C]    addiu $a2, $a1, 0//prev = next
[0x04000030]    addiu $a1, $a0, 0//next = final
[0x04000034]    bne $a3, $a5, -18//if branch taken jump to 0x04000018
[0x04000038]    addiu $v0, $zero, 0xA //set $v0 to 10 to exit
[0x0400003C]    syscall



HEX
[0x04000000]    24040000
[0x04000004]    24050001
[0x04000008]    24060000
[0x0400000C]    2407000a
[0x04000010]    24080001
[0x04000014]    24090000
[0x04000018]    25290000
[0x0400001C]    9120000C
[0x04000020]    24840001
[0x04000024]    08400018
[0x04000028]    00853040
[0x0400002C]    24C50000
[0x04000030]    29D3FFE7
[0x04000034]    2402000a
[0x04000038]    c

*/