#ifndef KH_TOKENIZER_H

// TODO(flo): we need to seriously improve this and implement a parser for our own stuff
// TODO(flo): better parsing for string, hex, int, float etc...
// take a look at : https://github.com/vurtun/mmx/blob/master/mm_sched.h
// TODO(flo): we certainly need to output cpp files (at least for debugging)
// http://www.tutorialspoint.com/data_structures_algorithms/expression_parsing.htm

enum TokenType
{
	Token_unknown,
	Token_open_parenthesis,
	Token_close_parenthesis,
	Token_colon,
	Token_semicolon,
	Token_asterisk,
	Token_open_bracket,
	Token_close_bracket,
	Token_open_brace,
	Token_close_brace,
	Token_equals,
	Token_plus,
	Token_minus,
	Token_slash,
	Token_backslash,
	Token_comma,
	Token_percentage,
	Token_at,
	Token_sharp,
	Token_dollar,
	Token_ampersand,
	Token_pipe,
	Token_exclamation,
	Token_question,
	Token_underscore,
	Token_greater,
	Token_less,
	Token_tilde,
	Token_caret,
	Token_backquote,



	Token_string,
	Token_word,
	Token_numeric,
	Token_decimal,

	Token_end_of_file,
};

struct Token
{
 	TokenType type;
	size_t text_length;
	char *text;
};

struct StringTokenizer
{
	// TokenType last_token_type;
	char *pos;
};

inline bool
end_of_line(char c)
{
	bool res = ((c == '\n') ||
		(c == '\r'));
	return(res);
}

inline bool
white_space(char c)
{
	bool res = ((c == ' ') ||
		(c == '\t') ||
		(c == '\v') ||
		(c == '\f') ||
		end_of_line(c));
	return(res);
}

inline bool
alphabetic(char c)
{
	bool res = (((c >= 'a') && (c <= 'z')) ||
		((c >= 'A') && (c <= 'Z')));
	return(res);
}

inline bool
numeric(char c)
{
	bool res = ((c >= '0') && (c <= '9'));
	return(res);
}

inline bool
word_fit(Token tok, char *word)
{
	char *pos = word;
	for(int ind = 0; ind < tok.text_length; ++ind, ++pos)
	{
		if((*pos == 0) ||
			(tok.text[ind] != *pos))
		{
			return(false);
		}
	}

	bool res = (*pos == 0);
	return(res);
}

KH_INTERN void
ignore_white_space(StringTokenizer *tokenizer)
{
	while(true)
	{
		if(white_space(tokenizer->pos[0]))
		{
			++tokenizer->pos;
		}
		else if((tokenizer->pos[0] == '/') &&
			(tokenizer->pos[1] == '/'))
		{
			tokenizer->pos += 2;
			while(tokenizer->pos[0] && !end_of_line(tokenizer->pos[0]))
			{
				++tokenizer->pos;
			}
		}
		else if((tokenizer->pos[0] == '/') &&
			(tokenizer->pos[1] == '*'))
		{
			tokenizer->pos += 2;
			while(tokenizer->pos[0] &&
				!((tokenizer->pos[0] == '*') &&
				(tokenizer->pos[1] == '/')))
			{
				++tokenizer->pos;
			}
			if(tokenizer->pos[0] == '*')
			{
				tokenizer->pos += 2;
			}
		}
		else
		{
			// TODO(flo): maybe handle #if 0 to #endif statements ?
			break;
		}
	}
}

KH_INTERN void
read_number(StringTokenizer *tokenizer, Token *tok)
{
	// TODO(flo): hexadecimal, octant, binaries
	tok->type = Token_numeric;

	u32 dot = 0;
	while(numeric(tokenizer->pos[0]) || (tokenizer->pos[0] == '.' && dot <= 1) || tokenizer->pos[0] == 'e')
	{
		if(tokenizer->pos[0] == '.')
		{
			tok->type = Token_decimal;
			++dot;
		}
		if(tokenizer->pos[0] == 'e')
		{
			tok->type = Token_decimal;
			++tokenizer->pos;
			if(tokenizer->pos[0] == '-' || tokenizer->pos[0] == '+')
			{

			}
			else
			{
				// NOTE(flo): some error messages here!
			}
		}
		++tokenizer->pos;
	}
}

KH_INTERN Token
get_token_and_next(StringTokenizer *tokenizer)
{
	ignore_white_space(tokenizer);

	Token res = {};
	res.text_length = 1;
	res.text = tokenizer->pos;
	char c = tokenizer->pos[0];
	++tokenizer->pos;

	switch(c)
	{
		case '\0': {res.type = Token_end_of_file;} break;
		case '(': {res.type = Token_open_parenthesis;} break;
		case ')': {res.type = Token_close_parenthesis;} break;
		case ':': {res.type = Token_colon;} break;
		case ';': {res.type = Token_semicolon;} break;
		case '*': {res.type = Token_asterisk;} break;
		case '[': {res.type = Token_open_bracket;} break;
		case ']': {res.type = Token_close_bracket;} break;
		case '{': {res.type = Token_open_brace;} break;
		case '}': {res.type = Token_close_brace;} break;
		case '=': {res.type = Token_equals;} break;
		case '+': {res.type = Token_plus;} break;
		case '-': {res.type = Token_minus;} break;
		case '\\' : {res.type = Token_backslash;} break;
		case '/' : {res.type = Token_slash;} break;
		case ',' : {res.type = Token_comma;} break;
		case '%' : {res.type = Token_percentage;} break;
		case '@' : {res.type = Token_at;} break;
		case '#' : {res.type = Token_sharp;} break;
		case '$' : {res.type = Token_dollar;} break;
		case '&' : {res.type = Token_ampersand;} break;
		case '|' : {res.type = Token_pipe;} break;
		case '!' : {res.type = Token_exclamation;} break;
		case '?' : {res.type = Token_question;} break;
		case '_' : {res.type = Token_underscore;} break;
		case '>' : {res.type = Token_greater;} break;
		case '<' : {res.type = Token_less;} break;
		case '~' : {res.type = Token_tilde;} break;
		case '^' : {res.type = Token_caret;} break;
		case '`' : {res.type = Token_backquote;} break;

		case '\"' :
		{
			res.type = Token_string;

			res.text = tokenizer->pos;

			while(tokenizer->pos[0] && tokenizer->pos[0] != '\"')
			{
				if((tokenizer->pos[0] == '\\') &&
					tokenizer->pos[1])
				{
					++tokenizer->pos;
				}
				++tokenizer->pos;
			}

			res.text_length = tokenizer->pos - res.text;
			if(tokenizer->pos[0] == '\"')
			{
				++tokenizer->pos;
			}
		} break;

		default:
		{
			if(alphabetic(c))
			{
				res.type = Token_word;

				while(alphabetic(tokenizer->pos[0]) || numeric(tokenizer->pos[0]) || tokenizer->pos[0] == '_')
				{
					++tokenizer->pos;
				}
				res.text_length = tokenizer->pos - res.text;
			}
			else if(numeric(c))
			{
				read_number(tokenizer, &res);
				res.text_length = tokenizer->pos - res.text;
			}
			else
			{
				res.type = Token_unknown;
			}
		} break;
	}

	return(res);
}

KH_INTERN bool
token_fit(Token t, TokenType desired_type)
{
	bool res = (t.type == desired_type);
	return(res);
}

KH_INTERN f64
token_to_f64(Token tok)
{
	kh_assert(token_fit(tok, Token_decimal));

	f64 res = 0.0;
	f64 mag;
	u32 len = 0;
	char *c = tok.text;

	i32 _pow, i;
	i32 _div = 0;
	while(len < tok.text_length && tok.text[len] != '.' && tok.text[len] != 'e')
	{
		res = res * 10.0 + (f64)(tok.text[len] - '0');
		len++;
	}

	if(len < tok.text_length && tok.text[len] == '.')
	{
		len++;
		for(mag = 0.1; len < tok.text_length; ++len, mag *= 0.1f)
		{
			if(tok.text[len] == 'e')
			{
				break;
			}
			res = res + (f64)(tok.text[len] - '0') * mag;
		}
	}

	if(len < tok.text_length && tok.text[len] == 'e')
	{
		len++;
		if(tok.text[len] == '-')
		{
			_div = 1;
			len++;
		}
		else if(tok.text[len] == '+')
		{
			len++;
		}

		for(_pow = 0; len < tok.text_length; ++len)
		{
			_pow = _pow * 10 + (i32)(tok.text[len] - '0'); 
		}
		for(mag = 1.0, i = 0; i < _pow; ++i)
		{
			mag *= 100.0;
		}
		if(_div)
		{
			res = res / mag;
		}
		else
		{
			res = res * mag;
		}
	}
	return(res);

}

KH_INTERN f32
token_to_f32(Token tok)
{
	f32 res = (f32)token_to_f64(tok);
	return(res);
}

KH_INTERN u32
token_to_u32(Token tok)
{
	kh_assert(token_fit(tok, Token_numeric));

	u32 res = 0;
	u32 len = 0;
	while(len < tok.text_length)
	{
		res = res * 10 + (u32)(tok.text[len] - '0');
		len++;
	}
	return(res);
}

#define KH_TOKENIZER_H
#endif
