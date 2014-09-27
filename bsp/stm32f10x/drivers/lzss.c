#include <dfs_init.h>
#include <dfs_elm.h>
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>

#define	N		128	// size of ring buffer 
#define NIL		N	// index for root of binary search trees 
/* upper limit for g_match_len. Changed from 18 to 16 for binary
compatability with Microsoft COMPRESS.EXE and EXPAND.EXE
#define	F		18 */
#define	F		16
#define	THRESHOLD	2

#define	READ_LE32(X)	*(uint32_t *)(X)
#define	WRITE_LE32(X,Y)	*(uint32_t *)(X) = (Y)

/* this assumes sizeof(long)==4 */

static u32 g_text_size, g_code_size;//, g_print_count;
static u8 g_ring_buffer[N + F - 1];
static u16 g_match_pos, g_match_len;
static u16 g_left_child[N + 1], g_right_child[N + 257], g_parent[N + 1];

//char g_ms_hdr[14] = "SZDDˆð'3A";
static void init_tree(void)
{
	unsigned i;
	for(i = N + 1; i <= N + 256; i++)
		g_right_child[i] = NIL;
	for(i = 0; i < N; i++)
		g_parent[i] = NIL;
}

static void insert_node(int r)
{
	u8 *key;
	u16 i, p;
	int cmp;

	cmp = 1;
	key = &g_ring_buffer[r];
	p = N + 1 + key[0];
	g_right_child[r] = g_left_child[r] = NIL;
	g_match_len = 0;
	while(1)
	{
		if(cmp >= 0)
		{
			if(g_right_child[p] != NIL)
				p = g_right_child[p];
			else
			{
				g_right_child[p] = r;
				g_parent[r] = p;
				return;
			}
		}
		else
		{
			if(g_left_child[p] != NIL)
				p = g_left_child[p];
			else
			{
				g_left_child[p] = r;
				g_parent[r] = p;
				return;
			}
		}
		for(i = 1; i < F; i++)
		{
			cmp = key[i] - g_ring_buffer[p + i];
			if(cmp != 0)
				break;
		}
		if(i > g_match_len)
		{
			g_match_pos = p;
			g_match_len = i;
			if(g_match_len >= F)
				break;
		}
	}
	g_parent[r] = g_parent[p];
	g_left_child[r] = g_left_child[p];
	g_right_child[r] = g_right_child[p];
	g_parent[g_left_child[p]] = r;
	g_parent[g_right_child[p]] = r;
	if(g_right_child[g_parent[p]] == p)
		g_right_child[g_parent[p]] = r;
	else
		g_left_child[g_parent[p]] = r;
	g_parent[p] = NIL;  /* remove p */
}
static void delete_node(unsigned p)
{
	unsigned q;

	if(g_parent[p] == NIL)
		return; /* not in tree */
	if(g_right_child[p] == NIL)
		q = g_left_child[p];
	else if(g_left_child[p] == NIL)
		q = g_right_child[p];
	else
	{
		q = g_left_child[p];
		if(g_right_child[q] != NIL)
		{
			do q = g_right_child[q];
			while(g_right_child[q] != NIL);
			g_right_child[g_parent[q]] = g_left_child[q];
			g_parent[g_left_child[q]] = g_parent[q];
			g_left_child[q] = g_left_child[p];
			g_parent[g_left_child[p]] = q;
		}
		g_right_child[q] = g_right_child[p];
		g_parent[g_right_child[p]] = q;
	}
	g_parent[q] = g_parent[p];
	if(g_right_child[g_parent[p]] == p)
		g_right_child[g_parent[p]] = q;
	else
		g_left_child[g_parent[p]] = q;
	g_parent[p] = NIL;
}

static void compress(const char *s_name, const char *d_name)
{
	u16 i, len, r, s, last_match_length, code_buf_ptr;
	u8 code_buf[17], mask;
	u8 c;
	u32 size;
    int infile, outfile;
    
    infile = open(s_name,O_RDWR,0x777);
    if (infile < 0)
        return;
    unlink(d_name);
    outfile = open(d_name,O_CREAT | O_RDWR,0x777);
    if (outfile < 0)
        return;
	get_file_size(s_name, &size);

//	WRITE_LE32(g_ms_hdr + 10, size);
//		if(fwrite(g_ms_hdr, 1, 14,outfile) != 14)
//		{
//			printf("Error writing output file \n");
//			fclose(infile);
//			fclose(outfile);			
//		}
	init_tree(); 
	code_buf[0] = 0;
	code_buf_ptr = mask = 1;
	s = 0;
	r = N - F;
	memset(g_ring_buffer + s, ' ', r - s);
	for(len = 0; len < F; len++)
	{
		if (!read(infile, &c, 1))
            break;
		g_ring_buffer[r + len] = c;
	}
	g_text_size = len;
	if(g_text_size == 0) 
		return;
	for(i = 1; i <= F; i++)
		insert_node(r - i);
	insert_node(r);
	do
	{
		if(g_match_len > len)
			g_match_len = len;
		if(g_match_len <= THRESHOLD)
		{
			g_match_len = 1;
			code_buf[0] |= mask;  
			code_buf[code_buf_ptr] = g_ring_buffer[r];  
			code_buf_ptr++;
		}
		else
		{
			code_buf[code_buf_ptr] = (unsigned char)g_match_pos;
			code_buf_ptr++;
			code_buf[code_buf_ptr] = (unsigned char)
				(((g_match_pos >> 4) & 0xF0) |
				(g_match_len - (THRESHOLD + 1)));
			code_buf_ptr++;
		}
		mask <<= 1;
		if(mask == 0)
		{
			for(i = 0; i < code_buf_ptr; i++)
                write(outfile, &code_buf[i], 1);
			g_code_size += code_buf_ptr;
			code_buf[0] = 0;
			code_buf_ptr = mask = 1;
		}
		last_match_length = g_match_len;
		for(i = 0; i < last_match_length; i++)
		{
            if (!read(infile, &c, 1))
                break;
			delete_node(s);
			g_ring_buffer[s] = c;
			if(s < F - 1)
				g_ring_buffer[s + N] = c;
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			insert_node(r);
		}
		g_text_size += i;
	//	if(g_text_size > g_print_count)
	//	{
	//		printf("%12ld\r", g_text_size);
	//		g_print_count += 1024;
	//	}
		while(i++ < last_match_length)
		{
			delete_node(s);		
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			len--;
			if(len)
				insert_node(r);
		}
	} while(len > 0); 
	
	if(code_buf_ptr > 1)
	{
		for(i = 0; i < code_buf_ptr; i++)
            write(outfile, &code_buf[i], 1);
		g_code_size += code_buf_ptr;
	}
    close(infile);
    close(outfile);
}

static void decompress(const char *s_name, const char *d_name)
{
	unsigned r, flags;
	u8 c, i, j, k;

    int infile, outfile;
    
    infile = open(s_name,O_RDWR,0x777);
    if (infile < 0)
        return;
    unlink(d_name);
    outfile = open(d_name,O_CREAT | O_RDWR,0x777);
    if (outfile < 0)
        return;
	memset(g_ring_buffer, ' ', N - F);
	r = N - F;
	for(flags = 0; ; flags >>= 1)
	{
		if((flags & 0x100) == 0)
		{
            if (!read(infile, &c, 1))
                break;
			flags = c | 0xFF00;
		}
		if(flags & 1)
		{
            if (!read(infile, &c, 1))
                break;
			write(outfile, &c, 1);
			g_ring_buffer[r] = c;
			r = (r + 1) & (N - 1);
		}
		else
		{
            if (!read(infile, &i, 1))
                break;
            if (!read(infile, &j, 1))
                break;
			i |= ((j & 0xF0) << 4);
			j = (j & 0x0F) + THRESHOLD;
			for(k = 0; k <= j; k++)
			{
				c = g_ring_buffer[(i + k) & (N - 1)];
				write(outfile, &c, 1);
				g_ring_buffer[r] = c;
				r = (r + 1) & (N - 1);
			}
		}
	}
    close(infile);
    close(outfile);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(compress, [s_name d_name]);
FINSH_FUNCTION_EXPORT(decompress, [s_name d_name]);
#endif