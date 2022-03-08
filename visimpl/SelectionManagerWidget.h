/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SELECTIONMANAGERWIDGET_H_
#define SELECTIONMANAGERWIDGET_H_

// Qt
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>

// C++
#include <unordered_set>

// ViSimpl
#include "types.h"

namespace visimpl
{
  class SelectionManagerWidget
  : public QWidget
  {
    Q_OBJECT;
  public:

    enum TSeparator
    {
      TSEP_NEWLINE = 0,
      TSEP_SPACE,
      TSEP_TAB,
      TSEP_OTHER
    };

    SelectionManagerWidget( QWidget* parent = nullptr );
    virtual ~SelectionManagerWidget( void ) {};

    void init( void );

    void setGIDs( const TGIDSet& all_,
                  const TGIDUSet& selected_ = { });

    void setSelected( const TGIDUSet& selected_ );
    const TGIDUSet& selected( void ) const;

    void clearSelection( void );

  signals:
    void selectionChanged( void );

  protected slots:
    void _addToSelected( void );
    void _removeFromSelected( void );

    void _buttonBrowseClicked( void );
    void _buttonSaveClicked( void );

    void _buttonCancelClicked( void );
    void _buttonAcceptClicked( void );

    void _updateRangeEdit();

  protected:
    void _initTabSelection( void );
    void _initTabExport( void );

    void _fillLists( void );
    void _reloadLists( void );

    void _updateListsLabelNumbers( void );

    void _saveToFile( const QString& filePath,
                      const QString& separator = "\n",
                      const QString& prefix = "",
                      const QString& suffix = "" );

    TGIDUSet _gidsAll;
    TGIDUSet _gidsSelected;
    TGIDUSet _gidsAvailable;

    QTabWidget* _tabWidget;

    // Selection tab
    QListView* _listViewAvailable;
    QListView* _listViewSelected;

    QStandardItemModel* _modelAvailable;
    QStandardItemModel* _modelSelected;

    QPushButton* _buttonAddToSelection;
    QPushButton* _buttonRemoveFromSelection;

    QLabel* _labelAvailable;
    QLabel* _labelSelection;
    QLineEdit *_rangeEdit;

    TUIntUintMap _gidIndex;

    // Export tab
    QLineEdit* _lineEditFilePath;
    QLineEdit* _lineEditPrefix;
    QLineEdit* _lineEditSuffix;
    QLineEdit* _lineEditSeparator;

    QRadioButton* _radioNewLine;
    QRadioButton* _radioSpace;
    QRadioButton* _radioTab;
    QRadioButton* _radioOther;

    QPushButton* _buttonBrowse;
    QPushButton* _buttonSave;

    QString _pathExportDefault;

  private:
    /** \struct Range
     * \brief Minimal class to represent a range of uint64_t and its basic operations.
     *
     */
    struct Range
    {
        unsigned long long min;
        unsigned long long max;

        /** \brief Range class constructor.
         * \param[in] a Range minimum
         * \param[in] b Range maximim
         */
        Range(unsigned long a, unsigned long b)
        : min { a }, max { b }
        {};

        /** \brief Returns true if the given Range can be merged.
         * \param[in] r Range reference.
         */
        bool canMerge(const Range &r)
        {
          // dont overcomplicate it, Qt returns QItemSelections without overlaps.
          return (max + 1 == r.min) || (min - 1 == r.max);
        };

        /** \brief Modifies the range struct adding with the given one.
         * \param[in] r Range reference.
         */
        Range& operator+(const Range &r)
        {
          if (max + 1 == r.min)
            max = r.max;
          else
            min = r.min;

          return *this;
        };

        /** \brief Returns a QString with the range.
         *
         */
        QString print() const
        {
          if(min != max) return QString("%1-%2").arg(min).arg(max);
          return QString("%1").arg(min);
        }
    };

    using Ranges = std::vector<Range>;

    /** \brief Returns the minimal vector of ranges representing the given one.
     * \param[inout] r Vector of ranges. Returned sorted.
     *
     */
    Ranges mergeRanges(Ranges &r);

    /** \brief Returns a QString with the given ranges separated by commas.
     *
     */
    QString printRanges(const Ranges &r);

  };
}

#endif /* SELECTIONMANAGERWIDGET_H_ */
