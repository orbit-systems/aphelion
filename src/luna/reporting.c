#include "common/util.h"
#include "common/ansi.h"

#include "reporting.h"
#include "luna.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>

// question not the dark magic that
// lights your home and keeps you safe

typedef struct GutterElement {
    usize pos;
    ReportKind report_kind;
    ReportLabelKind label_kind;
    bool extend;
} GutterElement;

typedef struct MultiLineEnding {
    ReportKind report_kind;
    ReportLabelKind label_kind;
    usize length;
    usize gutter_pos;
    string message;
} MultiLineEnding;

typedef struct MultiLineStart {
    ReportKind report_kind;
    ReportLabelKind label_kind;
    usize length;
    usize gutter_pos;
} MultiLineStart;

typedef struct SingleLine {
    ReportKind report_kind;
    ReportLabelKind label_kind;
    usize length;
    usize start;
    string message;
} SingleLine;

// return `true` if A intersects with B
static bool ranges_overlap(usize a1, usize a2, usize b1, usize b2) {
    int start = a1 > b1 ? a1 : b1;
    int end = a2 < b2 ? a2 : b2;

    return start < end;
}

// return `true` if A is fully contained in B
static bool ranges_contained(usize a1, usize a2, usize b1, usize b2) {
    return b1 <= a1 && a2 <= b2; 
}

static void render_header(
    FILE* out,
    ReportKind kind,
    string message
) {
    switch (kind) {
        case REPORT_ERROR: fprintf(out, Bold Red"error"Reset); break;
        case REPORT_WARNING: fprintf(out, Bold Yellow"warn"Reset); break;
    }
    fprintf(out, Bold": " str_fmt Reset"\n", str_arg(message));
}

static void render_snippet_start(
    FILE* out,
    string filename,
    usize line,
    usize col,
    usize outer_width
) {
    // for_n(i, 0, outer_width + 1) {
    //     fprintf(out, " ");
    // }
    // fprintf(out, Blue",- "Reset str_fmt":%zu:%zu\n", str_arg(filename), line, col);
    fprintf(out, Blue);
    
    for_n(i, 0, outer_width + 1) {
        fprintf(out, "-");
    }
    fprintf(out, "-> "  Reset str_fmt":%zu:%zu\n", str_arg(filename), line, col);
}

static usize digits_of_num(usize num) {
    if (num == 0) {
        return 1;
    }
    return 1 + (usize) log10((f64) num);
}

static void render_line(
    FILE* out,
    string source_line,
    usize line_num,
    usize outer_width,

    usize gutter_width,
    Vec(GutterElement) gutter_elems,

    Vec(MultiLineEnding) mline_endings,
    Vec(MultiLineStart) mline_starts,
    Vec(SingleLine) slines
) {

    // trim weird characters
    {
        while (source_line.len != 0) {
            switch (source_line.raw[0]) {
            case '\n':
            case '\r':
            case '\0':
                source_line.raw += 1;
                source_line.len -= 1;
                break;
            default:
                goto trim_end;
            }
        }
        trim_end:
        while (source_line.len != 0) {
            switch (source_line.raw[source_line.len - 1]) {
            case '\n':
            case '\r':
            case '\0':
                source_line.len -= 1;
                break;
            default:
                goto trim_done;
            }
        }
        trim_done:

    }

    // render line number and padding
    {
        usize line_num_digits = digits_of_num(line_num);
        assert(outer_width > line_num_digits);
        for_n(i, 0, outer_width - line_num_digits - 1) {
            fprintf(out, " ");
        }
        fprintf(out, Blue"%zu |"Reset, line_num);
        if (gutter_width > 0) {
            fprintf(out, " ");
        }
    }

    // render gutter elements
    for_n(i, 0, gutter_width) {
        GutterElement* elem = nullptr;
        for_n(j, 0, vec_len(gutter_elems)) {
            if (gutter_elems[j].pos == i) {
                elem = &gutter_elems[j];
                break;
            }
        }

        if (elem == nullptr) {
            fprintf(out, " ");
        } else {
            if (elem->label_kind == REPORT_LABEL_PRIMARY) {
                switch (elem->report_kind) {
                case REPORT_ERROR: fprintf(out, Red); break;
                case REPORT_WARNING: fprintf(out, Yellow); break;
                }
            } else {
                fprintf(out, Blue);
            }
            fprintf(out, "|"Reset);

        }
    }

    // render line
    fprintf(out, " "str_fmt"\n", str_arg(source_line));

    // render single line errors
    for_n(i, 0, vec_len(slines)) {
        for_n(j, 0, outer_width) {
            fprintf(out, " ");
        }
        fprintf(out, Blue"|"Reset);
        if (gutter_width > 0) {
            fprintf(out, " ");
        }

        // render gutter elements
        for_n(i, 0, gutter_width) {
            GutterElement* elem = nullptr;
            for_n(j, 0, vec_len(gutter_elems)) {
                if (gutter_elems[j].pos == i) {
                    elem = &gutter_elems[j];
                    break;
                }
            }
            if (elem == nullptr) {
                fprintf(out, " ");
            } else {
                if (elem->label_kind == REPORT_LABEL_PRIMARY) {
                    switch (elem->report_kind) {
                    case REPORT_ERROR: fprintf(out, Red); break;
                    case REPORT_WARNING: fprintf(out, Yellow); break;
                    }
                } else {
                    fprintf(out, Blue);
                }
                fprintf(out, "|"Reset);
            }
        }

        auto sline = slines[i];

        for_n(i, 0, sline.start + 1) {
            fprintf(out, " ");
        }

        if (sline.label_kind == REPORT_LABEL_PRIMARY) {
            switch (sline.report_kind) {
            case REPORT_ERROR:   fprintf(out, Red); break;
            case REPORT_WARNING: fprintf(out, Yellow); break;
            }
        } else {
            fprintf(out, Blue);
        }
        for_n(i, 0, sline.length) {
            if (sline.label_kind == REPORT_LABEL_PRIMARY) {
                fprintf(out, "^");
            } else {
                fprintf(out, "-");
            }
        }

        fprintf(out, " "str_fmt Reset"\n", str_arg(sline.message));
    }

    // render multiline endings
    for_n(i, 0, vec_len(mline_endings)) {
        auto ending = &mline_endings[i];

        for_n(j, 0, outer_width) {
            fprintf(out, " ");
        }
        fprintf(out, Blue"| "Reset);

        // render gutter
        for_n(j, 0, ending->gutter_pos) {
            for_n(k, 0, vec_len(mline_endings)) {
                if (mline_endings[k].gutter_pos == j) {
                    auto elem = &mline_endings[k];
                    fprintf(out, Blue"|"Reset);
                    goto gutter_render_finish;
                }
            }
            for_n(k, 0, vec_len(gutter_elems)) {
                if (gutter_elems[k].pos == j && gutter_elems->extend) {
                    auto elem = &gutter_elems[k];
                    fprintf(out, Blue"|"Reset);
                    goto gutter_render_finish;
                }
            }
            fprintf(out, " ");
            gutter_render_finish:
        }

        if (ending->label_kind == REPORT_LABEL_PRIMARY) {
            switch (ending->report_kind) {
            case REPORT_ERROR:   fprintf(out, Red); break;
            case REPORT_WARNING: fprintf(out, Yellow); break;
            }
        } else {
            fprintf(out, Blue);
        }
        fprintf(out, "`");

        for_n(j, ending->gutter_pos, gutter_width) {
            fprintf(out, "-");
        }

        for_n(i, 0, ending->length) {
            fprintf(out, "-");
        }
        if (ending->label_kind == REPORT_LABEL_PRIMARY) {
            fprintf(out, "^");
        } else {
            fprintf(out, "'");
        }
        fprintf(out, " "str_fmt Reset"\n", str_arg(ending->message));
        // destructive!
        ending->gutter_pos = USIZE_MAX - 5;
    }
    vec_len(mline_endings) = 0;

    // render multine starts
    for_n(i, 0, vec_len(mline_starts)) {
        auto start = &mline_starts[i];
        for_n(j, 0, outer_width) {
            fprintf(out, " ");
        }
        fprintf(out, Blue"| "Reset);

        // render gutter
        for_n(j, 0, start->gutter_pos) {
            for_n(k, 0, vec_len(mline_endings)) {
                if (mline_endings[k].gutter_pos == j) {
                    auto elem = &mline_endings[k];
                    if (elem->label_kind == REPORT_LABEL_PRIMARY) {
                        switch (elem->report_kind) {
                        case REPORT_ERROR:   fprintf(out, Red); break;
                        case REPORT_WARNING: fprintf(out, Yellow); break;
                        }
                    } else {
                        fprintf(out, Blue);
                    }
                    fprintf(out, "|"Reset);
                    goto gutter_render_finish2;
                }
            }
            for_n(k, 0, vec_len(gutter_elems)) {
                if (gutter_elems[k].pos == j && gutter_elems->extend) {
                    auto elem = &gutter_elems[k];
                    if (elem->label_kind == REPORT_LABEL_PRIMARY) {
                        switch (elem->report_kind) {
                        case REPORT_ERROR:   fprintf(out, Red); break;
                        case REPORT_WARNING: fprintf(out, Yellow); break;
                        }
                    } else {
                        fprintf(out, Blue);
                    }
                    fprintf(out, "|"Reset);
                    goto gutter_render_finish2;
                }
            }
            fprintf(out, " ");
            gutter_render_finish2:
        }

        if (start->label_kind == REPORT_LABEL_PRIMARY) {
            switch (start->report_kind) {
            case REPORT_ERROR:   fprintf(out, Red); break;
            case REPORT_WARNING: fprintf(out, Yellow); break;
            }
        } else {
            fprintf(out, Blue);
        }
        fprintf(out, ",");

        for_n(j, start->gutter_pos, gutter_width) {
            fprintf(out, "-");
        }

        for_n(i, 0, start->length) {
            fprintf(out, "-");
        }
        
        if (start->label_kind == REPORT_LABEL_PRIMARY) {
            fprintf(out, "^");
        } else {
            fprintf(out, "'");
        }
        fprintf(out, Reset"\n");
    }
}

void report_render(
    FILE* out,
    Report* r
) {
    render_header(out, r->kind, r->message);

    Vec(ReportLabel*) labels_of_file = vec_new(ReportLabel*, 16);

    Vec(string) lines = vec_new(string, 256);

    usize last_outer_width = 0;

    for_n(i, 0, vec_len(r->labels)) {
        ReportLabel* label = &r->labels[i];
        if (label->skip) {
            continue;
        }
        SourceFileId id = label->id;
        vec_clear(&labels_of_file);

        // now we have all the labels, we just have to do shit
        string source_text = (*r->sources)[id._].source;
        string source_path = (*r->sources)[id._].path;


        for_n(j, i, vec_len(r->labels)) {
            ReportLabel* label = &r->labels[j];

            if (label->id._ == id._) {
                label->skip = true;
                vec_append(&labels_of_file, label);
            }
        }


        vec_clear(&lines);

        ReportLabel* first_label = labels_of_file[0];

        bool snippet_start = false;
        usize line_count = 0;
        usize line_num = 0;
        usize col_num = 0;
        for_n(start, 0, source_text.len) {
            usize len = 0;
            while (start + len < source_text.len && source_text.raw[start + len] != '\n') {
                len += 1;
            }
            string line = {
                .len = len,
                .raw = &source_text.raw[start],
            };

            if (!snippet_start && ranges_overlap(start, start + len + 1, first_label->start, first_label->start + 1)) {
                col_num = label->start - start + 1;
                line_num = line_count + 1;
                snippet_start = true;
            }

            start += len; // skip newline we just found
            line_count += 1;

            // account for dumbass shit
            line.len += 1;

            vec_append(&lines, line);
        }

        usize outer_width = digits_of_num(vec_len(lines));

        usize single_line_labels = 0;

        usize max_gutter_width = 0;
        for_n(i, 0, vec_len(lines)) {
            string line = lines[i];

            usize start = (uintptr_t)line.raw - (uintptr_t)source_text.raw;
            usize len = line.len;


            for_n(i, 0, vec_len(labels_of_file)) {
                ReportLabel* label = labels_of_file[i];

                // if the label is fully contained in the line, it's a single-line label
                if (ranges_contained(label->start, label->end, start, start + len)) {
                    single_line_labels += 1;
                }

                // if the line is fully contained in the label, it's a gutter element
                if (ranges_contained( start, start + len, label->start, label->end - 1)) {
                    
                    if (label->gutter_pos == USIZE_MAX) {
                        max_gutter_width += 1;
                        label->gutter_pos -= 1;
                    }
                    continue;
                }

                // if the label only starts inside the line, it's a mline_start
                if ((start <= label->start && label->start < start + len) && 
                    !(start <= label->end && label->end < start + len) &&
                    !ranges_contained(label->start, label->end, start, start + len)
                ) {
                    if (label->gutter_pos == USIZE_MAX) {
                        max_gutter_width += 1;
                        label->gutter_pos -= 1;
                    }
                    continue;
                }

                // // if the label ends inside the line, it's a mline_ending
                if ((start <= label->end && label->end <= start + len) && !(start <= label->start && label->start <= start + len)) {
                    if (label->gutter_pos == USIZE_MAX) {
                        max_gutter_width += 1;
                        label->gutter_pos -= 1;
                    }
                    continue;
                }
            }
        }

        bool vertical_pad = single_line_labels != 1 || max_gutter_width != 0;

        render_snippet_start(out, source_path, line_num, col_num, outer_width);
        if (vertical_pad) {
            for_n(i, 0, outer_width + 1) {
                fprintf(out, " ");
            }
            fprintf(out, Blue"|"Reset"\n");
        }
        
        Vec(GutterElement) gutter_elems = vec_new(GutterElement, 16);
        Vec(MultiLineEnding) mline_endings = vec_new(MultiLineEnding, 16);
        Vec(MultiLineStart) mline_starts = vec_new(MultiLineStart, 16);
        Vec(SingleLine) slines = vec_new(SingleLine, 16);

        for_n(i, 0, vec_len(lines)) {
            string line = lines[i];

            usize start = (uintptr_t)line.raw - (uintptr_t)source_text.raw;
            usize len = line.len;
            
            // collect visual elements
            vec_clear(&gutter_elems);
            vec_clear(&mline_endings);
            vec_clear(&mline_starts);
            vec_clear(&slines);

            usize pos = max_gutter_width;

            bool at_least_one = false;
            for_n(i, 0, vec_len(labels_of_file)) {
                ReportLabel* label = labels_of_file[i];

                // if the label is fully contained in the line, it's a single-line label
                // printf("%zu - %zu\n%zu - %zu\n", label->start, label->end, start, start + len);
                if (ranges_contained(label->start, label->end, start, start + len)) {
                    SingleLine sline = {
                        .label_kind = label->kind,
                        .report_kind = r->kind,
                        .length = label->end - label->start,
                        .message = label->message,
                        .start = label->start - start,
                    };
                    vec_append(&slines, sline);
                    at_least_one = true;
                }

                // if the line is fully contained in the label, it's a gutter element
                if (ranges_contained( start, start + len, label->start, label->end - 1)) {
                    // find gutter slot
                    if (label->gutter_pos == USIZE_MAX - 1) {
                        label->gutter_pos = --pos;
                    }
                    assert(label->gutter_pos < max_gutter_width);
                    GutterElement gutter = {
                        .label_kind = label->kind,
                        .report_kind = r->kind,
                        .extend = true,
                        .pos = label->gutter_pos,
                    };
                    vec_append(&gutter_elems, gutter);
                    at_least_one = true;
                }

                // if the label only starts inside the line, it's a mline_start
                if ((start <= label->start && label->start < start + len) && 
                    !(start <= label->end && label->end < start + len) &&
                    !ranges_contained(label->start, label->end, start, start + len)
                ) {
                    // find gutter slot
                    if (label->gutter_pos == USIZE_MAX - 1) {
                        label->gutter_pos = --pos;

                    }
                    assert(label->gutter_pos < max_gutter_width);
                    MultiLineStart mline = {
                        .gutter_pos = label->gutter_pos,
                        .label_kind = label->kind,
                        .report_kind = r->kind,
                        .length = label->start - start,
                    };
                    vec_append(&mline_starts, mline);

                    at_least_one = true;
                }

                // if the label ends inside the line, it's a mline_ending
                if ((start <= label->end && label->end <= start + len) && !(start <= label->start && label->start <= start + len)) {
                    // find gutter slot
                    if (label->gutter_pos == USIZE_MAX - 1) {
                        label->gutter_pos = --pos;

                    }
                    assert(label->gutter_pos < max_gutter_width);
                    GutterElement gutter = {
                        .label_kind = label->kind,
                        .report_kind = r->kind,
                        .extend = true,
                        .pos = label->gutter_pos,
                    };
                    vec_append(&gutter_elems, gutter);
                    MultiLineEnding mline = {
                        .gutter_pos = label->gutter_pos,
                        .label_kind = label->kind,
                        .report_kind = r->kind,
                        .length = label->end - start,
                        .message = label->message,
                    };
                    vec_append(&mline_endings, mline);

                    at_least_one = true;
                }
            }
            if (at_least_one) {
                render_line(out, 
                    line, 
                    i + 1, 
                    outer_width + 1, 
                    max_gutter_width,
                    gutter_elems, 
                    mline_endings, 
                    mline_starts, 
                    slines
                );
            }
        }

        last_outer_width = outer_width;
        
        vec_destroy(&gutter_elems);
        vec_destroy(&mline_endings);
        vec_destroy(&mline_starts);
        vec_destroy(&slines);
        
        if (vertical_pad) {
            for_n(i, 0, outer_width + 1) {
                fprintf(out, " ");
            }
            fprintf(out, Blue"|"Reset"\n");
        }
    }


    for_n(i, 0, vec_len(r->notes)) {
        string note = r->notes[i];

        for_n(i, 0, last_outer_width + 1) {
            fprintf(out, " ");
        }
        fprintf(out, Blue"="Reset" ");
        for_n(i, 0, note.len) {
            fputc(note.raw[i], out);
            if (note.raw[i] == '\n') {
                for_n(i, 0, last_outer_width + 3) {
                    fprintf(out, " ");
                }
            }
        }
        fputc('\n', out);
    }

    vec_destroy(&lines);
    vec_destroy(&labels_of_file);
}

Report* report_new(
    ReportKind kind, 
    string message,
    Vec(SourceFile) const* sources
) {
    Report* r = malloc(sizeof(*r));
    r->kind = kind;
    r->message = string_clone(message);
    r->labels = vec_new(ReportLabel, 4);
    r->notes = vec_new(string, 4);
    r->sources = sources;

    return r;
}
void report_destroy(Report* r) {
    for_n(i, 0, vec_len(r->labels)) {
        string_free(r->labels[i].message);
    }

    vec_destroy(&r->labels);
    vec_destroy(&r->notes);
    string_free(r->message);
    *r = (Report){};
}

void report_add_label(Report* r,
    ReportLabelKind kind,
    SourceFileId id,
    usize start, 
    usize end,
    string message
) {
    ReportLabel label = {
        .kind = kind,
        .id = id,
        .start = start,
        .end = end,
        .message = string_clone(message),
        .skip = false,
        .gutter_pos = USIZE_MAX,
    };

    vec_append(&r->labels, label);
}

void report_add_note(Report* r, string note) {
    vec_append(&r->notes, note);
}
