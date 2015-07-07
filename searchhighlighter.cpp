/**
 * \file
 * <!--
 * Copyright 2015 Develer S.r.l. (http://www.develer.com/)
 * -->
 *
 * \brief cutecom-ng search result highliter implementation
 *
 * \author Aurelien Rainone <aurelien@develer.org>
 */

#include "searchhighlighter.h"

SearchHighlighter::SearchHighlighter(QTextDocument *parent, bool has_cursor_) :
    QSyntaxHighlighter(parent),
    has_cursor(has_cursor_),
    _num_occurences(0),
    _occurence_cursor(0),
    last_cursor_pos(0)
{
}

void SearchHighlighter::setSearchString(const QString &search)
{
    // reset all before highlight loop
    _search_string = search;
    _occurence_cursor = _num_occurences = occurence_pos = 0;
    search_string_changed = true;
    rehighlight();
    if (has_cursor)
    {
        if (search_string_changed && _num_occurences > 0)
        {
            // last highlight loop, we didn't mark nothing because the cursor
            // should be positionned before the previous position
            // so now we rest it and redo the loop
            last_cursor_pos = 0;
            setSearchString(search);
        }
        if (search.isEmpty())
            emit totalOccurencesChanged(-1);
        else
            emit totalOccurencesChanged(_num_occurences);
    }
}

void SearchHighlighter::highlightBlock(const QString &text)
{
    // highlighted text background color (search results)
    static const Qt::GlobalColor SEARCHRESULT_BACKCOL = Qt::yellow;

    // highlighted text background color (search results)
    static const Qt::GlobalColor CURSOR_SEARCHRESULT_BACKCOL = Qt::red;

    if (!_search_string.isEmpty())
    {
        if (!text.isEmpty())
        {
            QTextCharFormat charFormat;
            charFormat.setBackground(SEARCHRESULT_BACKCOL);

            const int length = _search_string.length();
            int index = text.indexOf(_search_string, 0, Qt::CaseInsensitive);
            while (index >= 0)
            {
                charFormat.setBackground(SEARCHRESULT_BACKCOL);

                if (has_cursor)
                {
                    if (search_string_changed)
                    {
                        if (occurence_pos + index >= last_cursor_pos)
                        {
                            // if search_string just changed, _occurence_cursor has been invalidated
                            // because it is bound to a specific search string
                            // however last_cursor_pos records the position
                            // so now we highlight the first search result we find
                            last_cursor_pos = occurence_pos + index;
                            _occurence_cursor = _num_occurences;
                            charFormat.setBackground(CURSOR_SEARCHRESULT_BACKCOL);
                            search_string_changed = false;

                            emit cursorPosChanged(occurence_pos + index);
                        }
                    }
                    else if (_num_occurences == _occurence_cursor)
                    {
                        // record position of this occurence
                       last_cursor_pos = occurence_pos + index;
                        charFormat.setBackground(CURSOR_SEARCHRESULT_BACKCOL);

                        emit cursorPosChanged(occurence_pos + index);
                    }
                }

                setFormat(index, length, charFormat);

                index = text.indexOf(_search_string, index + length, Qt::CaseInsensitive);
                ++_num_occurences;
            }

            occurence_pos += text.length();
        }
    }
}

void SearchHighlighter::previousOccurence()
{
    // nothing to do if the string-search returned nothing
    if (!_search_string.isEmpty())
    {
        // cyclic behaviour
        if (_occurence_cursor == 0)
            _occurence_cursor = _num_occurences - 1;
        else
            --_occurence_cursor;
        _num_occurences = 0;
        occurence_pos = 0;

        rehighlight();
    }
}

void SearchHighlighter::nextOccurence()
{
    if (!_search_string.isEmpty())
    {
        // cyclic behaviour
        if (_occurence_cursor == _num_occurences - 1)
            _occurence_cursor = 0;
        else
            ++_occurence_cursor;
        _num_occurences = 0;
        occurence_pos = 0;

        rehighlight();
    }
}

int SearchHighlighter::cursorOccurence() const
{
    return _occurence_cursor;
}

int SearchHighlighter::totalOccurences() const
{
    return _num_occurences;
}
