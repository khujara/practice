KH_INTERN void
generate_material_header(char *src_file, char *out_file) {
	LinearArena tmp_arena = {};

	FileHandle src_file_hdl = g_platform.open_file(src_file, FileAccess_read, FileCreation_only_open);	
	kh_assert(!src_file_hdl.error);
	u32 src_size = g_platform.get_file_size(&src_file_hdl);
	char *file_contents = (char *)kh_push(&tmp_arena, src_size);
	g_platform.read_bytes_of_file(&src_file_hdl, 0, src_size, file_contents);
	g_platform.close_file(&src_file_hdl);

	FileHandle dst_file_hdl = g_platform.open_file(out_file, FileAccess_write, FileCreation_override);
	kh_assert(!dst_file_hdl.error);
	u32 offset = 0;

	char *enum_open = "enum MaterialType {\n\tType_Material_none,\n";
	char *enum_close = "\tType_Material_count,\n};\n\n";

	offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(enum_open), enum_open);

	char tmp_buf[512];
	StringTokenizer str_tok = {file_contents};
	Token tok = get_token_and_next(&str_tok);
	while(!token_fit(tok, Token_end_of_file)) {
		if(token_fit(tok, Token_colon)) {
			char comma = ',';
			char newline = '\n';
			tok = get_token_and_next(&str_tok);

			char *name = tok.text;
			umm name_len = tok.text_length;

			u32 len = kh_printf(tmp_buf, "\tType_%.*s", name_len, name);

			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, len, tmp_buf);
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &comma);
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &newline);

			while(!token_fit(tok, Token_close_brace)) {
				tok = get_token_and_next(&str_tok);
			}
		}
		tok = get_token_and_next(&str_tok);
	}

	offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(enum_close), enum_close);

	char *str_open = "char *G_MATERIAL_TYPES[Type_Material_count] = {\n\t\"Material_none\",\n";
	char *str_close = "};\n\n";

	offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(str_open), str_open);

	str_tok.pos = file_contents;
	tok = get_token_and_next(&str_tok);
	while(!token_fit(tok, Token_end_of_file)) {
		if(token_fit(tok, Token_colon)) {
			char comma = ',';
			char newline = '\n';
			tok = get_token_and_next(&str_tok);

			char *name = tok.text;
			umm name_len = tok.text_length;

			u32 len = kh_printf(tmp_buf, "\t\"%.*s\"", name_len, name);

			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, len, tmp_buf);
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &comma);
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &newline);

			while(!token_fit(tok, Token_close_brace)) {
				tok = get_token_and_next(&str_tok);
			}
		}
		tok = get_token_and_next(&str_tok);
	}

	offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(str_close), str_close);

	str_tok.pos = file_contents;
	tok = get_token_and_next(&str_tok);

	char *none_struct = "struct Material_none {\n};\n\n";
	offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(none_struct), none_struct);

	while(!token_fit(tok, Token_end_of_file)) {
		if(token_fit(tok, Token_colon)) {
			char *struct_header = "struct ";
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(struct_header), struct_header);
			tok = get_token_and_next(&str_tok);
			char *name = tok.text;
			umm name_len = tok.text_length;
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, name_len, name);
			char *struct_open = " {\n";
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(struct_open), struct_open);
			tok = get_token_and_next(&str_tok);
			kh_assert(token_fit(tok, Token_open_brace));
			tok = get_token_and_next(&str_tok);
			while(!token_fit(tok, Token_close_brace)) {
				char *param_type = tok.text;
				umm param_type_len = tok.text_length;

				char tab = '\t';
				char space = ' ';
				char semicolon = ';';
				char newline = '\n';
				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &tab);
				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, param_type_len, param_type);
				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &space);
				tok = get_token_and_next(&str_tok);

				char *param_name = tok.text;
				umm param_name_len = tok.text_length;

				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, param_name_len, param_name);
				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &semicolon);
				offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, 1, &newline);

				tok = get_token_and_next(&str_tok);
				kh_assert(token_fit(tok, Token_semicolon));
				tok = get_token_and_next(&str_tok);
			}

			char *struct_close = "};\n\n";
			offset += g_platform.write_bytes_to_file(&dst_file_hdl, offset, string_length(struct_close), struct_close);

		}

		tok = get_token_and_next(&str_tok);
	}

	g_platform.close_file(&dst_file_hdl);

	kh_clear(&tmp_arena);
}
