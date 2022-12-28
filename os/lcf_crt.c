str8 stdio_load_entire_file(Arena *arena, str8 filepath) {
    str8 file_content = {0};

    FILE *file = fopen(filepath.str, "rb");
    if (file != 0) {
        fseek(file, 0, SEEK_END);
        u64 file_len = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_content.str = (chr8*) Arena_take(arena, file_len+1);
        if (file_content.str != 0) {
            file_content.len = file_len;
            fread(file_content.str, 1, file_len, file);
            file_content.str[file_content.len] = 0;
        }
        fclose(file);
    }
    return file_content;
}

b32 stdio_write_file(str8 filepath, Str8List text) {
    u64 bytes_written = 0;
    FILE *file = fopen(filepath.str, "wb");
    if (file != 0) {
        Str8Node* n = text.first;
        for (s64 i = 0; i < text.count; i++, n = n->next) {
            if (!fwrite(n->str.str, n->str.len, 1, file)) {
                break;
            }
            bytes_written += n->str.len;
        }
        fclose(file);
    }
    return bytes_written == text.total_len;
}
