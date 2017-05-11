#ifndef KH_STRINGS_H

// TODO(flo): try to keep strings upon frame without much tracking
// do not allocate/deallocate all the time (temp buffers to no copying)
// offline preprocessing or something

inline u32
string_length(char *string)
{
	u32 count = 0;
	while(*string++)
	{
		++count;
	}
	return(count);
}

KH_INTERN void
strings_concat(usize a_length, char *a, usize b_length, char *b, usize dst_length, char *dst)
{
	kh_assert((a_length + b_length) < dst_length);

	for(u32 ind = 0; ind < a_length;	++ind)
	{
		*dst++ = *a++;
	}

	for(u32 ind = 0; ind < b_length;	++ind)
	{
		*dst++ = *b++;
	}

	*dst++ = 0;
}

KH_INTERN void
strings_concat(usize src_length, char *src, usize dst_length, char *dst)
{
	for(u32 ind = 0; ind < (src_length - 1); ++ind)
	{
		char *dst_at = dst + dst_length + ind;
		char *src_at = src + ind;
		*dst_at = *src_at;
	}
}

KH_INTERN void
strings_concat_at_offset(usize offset, usize a_length, char *a, usize b_length, char *b)
{
	kh_assert((offset + b_length) < a_length);

	usize end = offset + b_length;

	char *dst = a + offset;
	for(usize ind = offset; ind < end; ++ind)
	{
		*dst++ = *b++;
	}

	*dst++ = 0;
}

KH_INTERN void
strings_copy(usize a_length, char *a, char *dst)
{
	for(u32 ind = 0; ind < a_length; ++ind)
	{
		dst[ind] = a[ind];
	}
	dst[a_length] = 0;
}

KH_INTERN void
strings_copy_NNT(usize a_length, char *a, char *dst) {
	for(u32 ind = 0; ind < a_length; ++ind) {
		dst[ind] = a[ind];
	}
}

inline b32
strings_equals_on_size(u32 size, char *a, char *b)
{
	b32 res = true;
	if(a && b)
	{
		for(u32 i = 0; i < size; ++i, ++a, ++b)
		{
			if((*a != *b) || !*a || !*b)
			{
				res = false;
				break;
			}
		}
	}
	else
	{
		res = false;
	}
	return(res);
}

inline b32
strings_equals(char *a, char *b)
{
	b32 res = (a == b);

	if(a && b)
	{
		while(*a && *b && (*a == *b))
		{
			++a;
			++b;
		}
		res = ((*a == 0) && (*b == 0));
	}
	return(res);
}


void reverse_string(char *str, u32 length)
{
    u32 start = 0;
    u32 end = length -1;
    while (start < end)
    {
        char *tmp_start = str + start;
        char *tmp_end = str + end;

        char tmp = *tmp_start;
        *tmp_start = *tmp_end;
        *tmp_end = tmp;

        start++;
        end--;
    }
}
 
KH_INTERN void
i32_to_string(int num, char* str, int base = 10)
{
    int i = 0;
    bool is_negative = false;
 
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
 
    if (num < 0 && base == 10)
    {
        is_negative = true;
        num = -num;
    }
 
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9) ? (char)((rem-10) + 'a') : (char)(rem + '0');
        num = num/base;
    }
 
    if (is_negative)
        str[i++] = '-';
 
    str[i] = '\0';
 
    reverse_string(str, i);
}

// TODO(flo): kh_printf()




#define KH_STRINGS_H
#endif
