#ifndef LUNA_REPORTING_H
#define LUNA_REPORTING_H

// taken pretty much wholesale from the mars repository

#include "common/vec.h"
#include "common/str.h"
#include "common/type.h"

#include "luna.h"

#include <stdio.h>

/// Kind/severity of a report.
typedef enum ReportKind: u8 {
    REPORT_ERROR,
    REPORT_WARNING,
} ReportKind;

/// Kind/purpose of label inside of a report.
typedef enum ReportLabelKind: u8 {
    REPORT_LABEL_PRIMARY,
    REPORT_LABEL_SECONDARY,
} ReportLabelKind;

/// A span of source text with a message and other data.
/// The first primary label (or the first label, if there are none)
/// will be used as the 'main' source location.
typedef struct ReportLabel {
    ReportLabelKind kind;
    bool skip;
    SourceFileId id;
    usize gutter_pos;
    usize start, end;
    string message;
} ReportLabel;

/// A diagnostic report.
typedef struct Report {
    /// Severity of the report.
    ReportKind kind;
    /// Overall message for the report.
    string message;
    /// List of source labels.
    Vec(ReportLabel) labels;
    /// List of non-owned note strings.
    Vec(string) notes;
    /// Pointer to a list of source files.
    /// \note Report *does not own* the vector or its contents,
    ///       but it is expected to live as long as the report.
    Vec(SourceFile) const* sources;
} Report;

/// Create a new report and provide a reference to the list
/// of source files that will be used when printing.
/// \param kind Severity of the report.
/// \param message Overall message for the report.
/// \param sources Pointer to a list of source files.
/// \note Report *does not own* `sources` or its contents,
///       but it is expected to live as long as the report.
Report* report_new(
    ReportKind kind,
    string message, 
    Vec(SourceFile) const* sources
);

/// Destroy and deallocate a report.
void report_destroy(Report* r);

/// Add a label to the report `r`.
/// \param r Report being added to.
/// \param kind New label's kind.
/// \param id ID of the source file the label is contained in.
/// \param start Start of the label span, in bytes.
/// \param end End of the label span (exclusive), in bytes.
/// \param message Message to be displayed on the label.
void report_add_label(
    Report* r,
    ReportLabelKind kind,
    SourceFileId id,
    usize start, 
    usize end,
    string message
);

/// Add a short note to be printed after the source labels.
void report_add_note(Report* r, string note);

/// Render the report to a <stdio.h> file
/// \todo maybe change this to a string builder of some sort?
void report_render(
    FILE* out, 
    Report* r
);

#endif // LUNA_REPORTING_H
