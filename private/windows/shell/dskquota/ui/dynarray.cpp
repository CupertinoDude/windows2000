///////////////////////////////////////////////////////////////////////////////
/*  File: dynarray.cpp

    Description: Wrapper classes around the DPA_xxxxx and DSA_xxxxx functions 
        provided by the common control's library.  The classes add value by 
        providing multi-threaded protection, iterators and automatic cleanup 
        semantics.  

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
    09/03/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
#include "pch.h" // PCH
#pragma hdrstop

#include "dynarray.h"



///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::PointerList

    Description: Constructor.  

    Arguments:
        cItemGrow - Number of items to grow list when expansion is required.
                    Default value is 0 which causes DPA to use 8.

    Returns: Nothing.
    
    Exception: Throws OutOfMemory.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    09/03/96    Initial creation.                                    BrianAu
    02/21/97    Added cItemGrow argument.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
PointerList::PointerList(
    INT cItemGrow
    )
      : m_hdpa(NULL)
{
    InitializeCriticalSection(&m_cs);
    if (NULL == (m_hdpa = DPA_CreateEx(cItemGrow, NULL)))
    {
        DeleteCriticalSection(&m_cs);
        throw CAllocException();
    }
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::~PointerList

    Description: Destructor.  Destroys the DPA and closes the mutex handle.

    Arguments: None.

    Returns: Nothing.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
PointerList::~PointerList(
    VOID
    )
{
    Lock();
    if (NULL != m_hdpa)
        DPA_Destroy(m_hdpa);

    ReleaseLock();
    DeleteCriticalSection(&m_cs);
}



///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Count

    Description: Returns the number of elements in the list.

    Arguments: None.

    Returns: Count of elements in list.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
UINT 
PointerList::Count(
    VOID
    )
{
    UINT n = 0;

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    n = DPA_GetPtrCount(m_hdpa);

    return n;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Insert

    Description: Inserts a pointer into the pointer list at a given index.
        If the index is beyond the upper bounds of the array, the array
        is extended by one and the item is appended to the list.

    Arguments:
        pvItem - Pointer value to add to list.

        index - List index where pointer is to be inserted.  All following
            items are shifted to one index higher.  The list automatically
            grows to accomodate as required.

    Returns: Nothing.

    Exceptions: OutOfMemory.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
    09/03/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
VOID 
PointerList::Insert(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((0 <= index));

    if (DPA_InsertPtr(m_hdpa, index, pvItem) < 0)
    {
        throw CAllocException();
    }
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Replace

    Description: Replaces a pointer in the pointer list at a given index.
        If the index is beyond the upper bounds of the array, the array
        is extended by one and the item is appended to the list.

    Arguments:
        pvItem - Pointer value to add to list.

        index - List index where pointer is to be replaced.
          
    Returns:
        TRUE  - Success.
        FALSE - Invalid index or empty container.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::Replace(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((0 <= index));

    return DPA_SetPtr(m_hdpa, index, pvItem);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Append

    Description: Appends a pointer to the end of the list.

    Arguments:
        pvItem - Pointer value to add to list.

    Returns: Nothing.

    Exceptions: OutOfMemory.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
    09/03/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
VOID
PointerList::Append(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    //
    // Yes, this is correct.  We're "inserting" an item to append something
    // to the list.  This saves a ( count - 1 ) calculation.
    //
    DBGASSERT((NULL != m_hdpa));
    Insert(pvItem, DPA_GetPtrCount(m_hdpa));
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Remove

    Description: Removes a pointer from the list at a given index.

    Arguments: 
        ppvItem - Address of variable to contain removed pointer value.

        index - List index where pointer is to be removed.  All following
            items are shifted to one index lower. 

    Returns:
        TRUE  - Success.
        FASLE - Index is invalid (or container is empty).

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::Remove(
    LPVOID *ppvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != ppvItem));
    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((0 <= index));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    *ppvItem = DPA_DeletePtr(m_hdpa, index);
    if (NULL == *ppvItem)
        return FALSE;

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::RemoveLast

    Description: Removes the last pointer from the list.

    Arguments: 
        ppvItem - Address of variable to contain removed pointer value.
            All following items are shifted to one index lower. 

    Returns: 
        TRUE  - Success.
        FALSE - Container is emtpy.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL 
PointerList::RemoveLast(
    LPVOID *ppvItem
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    if (0 == Count())
        return FALSE;

    Remove(ppvItem, Count() - 1);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Retrieve

    Description: Retrieve a pointer from the list at a given index.
        The pointer value is merely retrieved and not removed from the list.

    Arguments:
        ppvItem - Address of variable to contain retrieved pointer value.

        index - List index where pointer is to be retrieved.  

    Returns: 
        TRUE  - Success.
        FALSE - Invalid index or container is empty.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::Retrieve(
    LPVOID *ppvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    *ppvItem = DPA_GetPtr(m_hdpa, index);

    if (NULL == *ppvItem)
        return FALSE;

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::RetrieveLast

    Description: Retrieves the last pointer from the list.

    Arguments: 
        ppvItem - Address of variable to contain retrieved pointer value.

    Returns:
        TRUE  - Success.
        FALSE - Container is empty.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
    09/03/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::RetrieveLast(
    LPVOID *ppvItem
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    if (0 == Count())
        return FALSE;

    Retrieve(ppvItem, Count() - 1);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::FindIndex

    Description: Returns the list index associated with a given pointer
        value.  If duplicates exist, the index of the first item is returned.

    Arguments: 
        pvItem - Pointer value of item to be found.

        pIndex - Address of index variable to hold resulting index.

    Returns:
        TRUE  = Success
        FALSE = Item not found in list.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
    09/03/96    Changed returned value to BOOL.                      BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::FindIndex(
    LPVOID pvItem, 
    INT *pIndex
    )
{
    INT i = -1;

    DBGASSERT((NULL != pIndex));
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    i = *pIndex = DPA_GetPtrIndex(m_hdpa, pvItem);

    if (-1 == i)
        return FALSE;

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Sort

    Description: Sorts the list given a comparison function.

    Arguments: 
        pfnCompare - Address of comparison callback.

        lParam - 32-bit parameter passed to the callback.

    Returns:
        TRUE  = Success
        FALSE = Item not found in list.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    02/21/97    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
PointerList::Sort(
    PFNDPACOMPARE pfnCompare, 
    LPARAM lParam
    )
{
    DBGASSERT((NULL != pfnCompare));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((NULL != pfnCompare));

    return DPA_Sort(m_hdpa, pfnCompare, lParam);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerList::Search

    Description: Searches the list for a given item.

    Arguments: 
        pvKey - Address of key item used for search.

        pfnCompare - Address of comparison callback.

        uOptions - Options controlling sort operation.
            DPAS_SORTED       = Array is already sorted.  
                                Will use binary search.
            DPAS_INSERTBEFORE = If no exact match is found, return index of
                                previous best match.
            DPAS_INSERTAFTER  = If no exact match is found, return index of
                                next best match.

        iStart - Index of where to start search.  0 for start of list.

        lParam - 32-bit parameter passed to the callback.

    Returns: Index of found item or -1 if none found.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    02/21/97    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
INT
PointerList::Search(
    LPVOID pvKey,
    PFNDPACOMPARE pfnCompare, 
    UINT uOptions,
    INT iStart,
    LPARAM lParam
    )
{
    DBGASSERT((NULL != pfnCompare));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will automatically release.

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((NULL != pvKey));
    DBGASSERT((NULL != pfnCompare));

    return DPA_Search(m_hdpa, pvKey, iStart, pfnCompare, lParam, uOptions);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerListIterator::operator = 

    Description: Assignment for PointerListIterator.

    Arguments:
        rhs - Reference to constant iterator that is the rhs of the assignment.

    Returns:
        Returns a reference to "this" iterator object following the assignment.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    02/27/97    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
PointerListIterator&
PointerListIterator::operator = (
    const PointerListIterator& rhs
    )
{
    if (this != &rhs)
    {
        m_pList = rhs.m_pList;
        m_Index = rhs.m_Index;
    }
    return *this;
}



///////////////////////////////////////////////////////////////////////////////
/*  Function: PointerListIterator::Advance

    Description: Both the Next() and Prev() iterator functions call this
        one function.  It handles the actual iteration.

    Arguments:
        ppvOut - Address of pointer variable to contain the value of the 
            pointer at the "current" iterator location.  The iterator is
            advance (or retreated) after the pointer value is copied to the
            destination.

        bForward - TRUE  = Advance toward end of list.
                   FALSE = Advance toward front of list.

    Returns:
        NO_ERROR      - Iterator advanced.  Returned pointer is valid.
        E_FAIL        - Iterator already at begining or end of list.  Returned
                        pointer will be NULL.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/14/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
HRESULT 
PointerListIterator::Advance(
    LPVOID *ppvOut, 
    BOOL bForward
    )
{
    LPVOID pv       = NULL;
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != ppvOut));

    m_pList->Lock();
    if (0 < m_pList->Count() && m_Index != EndOfList)
    {
        //
        // Get pointer value at index "m_Index".
        //
        DBGASSERT((NULL != m_pList->m_hdpa));
        pv = DPA_GetPtr(m_pList->m_hdpa, m_Index);

        if (bForward)
        {
            //
            // Advance iterator index.
            //
            if ((UINT)(++m_Index) == m_pList->Count())
                m_Index = EndOfList;
        }
        else
        {
            //
            // Retreat iterator index.
            //
            m_Index--;  // Will be -1 (EndOfList) if currently 0.
        }
    }
    else
        hResult = E_FAIL;

    m_pList->ReleaseLock();

    *ppvOut = pv;  // Return pointer value.

    return hResult;
}



///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::StructureList

    Description: Constructor.

    Arguments:
        cbItem - Size of each item in bytes.

        cItemGrow - Number of items to grow array at each expansion.


    Returns: Nothing.

    Exceptions: OutOfMemory

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    09/06/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
StructureList::StructureList(
    INT cbItem, 
    INT cItemGrow
    )
{
    InitializeCriticalSection(&m_cs);

    if (NULL == (m_hdsa = DSA_Create(cbItem, cItemGrow)))
    {
        DeleteCriticalSection(&m_cs);
        throw CAllocException();
    }
}




///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::~StructureList

    Description: Destructor.  Destroys the DSA and closes the mutex handle.

    Arguments: None.

    Returns: Nothing.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
StructureList::~StructureList(void)
{
    Lock();

    if (NULL != m_hdsa)
        DSA_Destroy(m_hdsa);

    ReleaseLock();
    DeleteCriticalSection(&m_cs);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureListIterator::operator = 

    Description: Assignment for StructureListIterator.

    Arguments:
        rhs - Reference to constant iterator that is the rhs of the assignment.

    Returns:
        Returns a reference to "this" iterator object following the assignment.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    02/27/97    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
StructureListIterator&
StructureListIterator::operator = (
    const StructureListIterator& rhs
    )
{
    if (this != &rhs)
    {
        m_pList = rhs.m_pList;
        m_Index = rhs.m_Index;
    }
    return *this;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Count

    Description: Returns the number of elements in the list.

    Arguments: None.

    Returns: Count of elements in list.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
UINT StructureList::Count(VOID)
{
    UINT n = 0;

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.

    DBGASSERT((NULL != m_hdsa));
    n = DSA_GetItemCount(m_hdsa);

    return n;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Insert

    Description: Insert an item into the Structure list at a given index.
        If the index is beyond the upper bounds of the array, the array
        is extended by one and the item is appended to the list.

    Arguments:
        pvItem - Address of item to add to list.

        index - List index where item is to be inserted.  All following
            items are shifted to one index higher.  The list automatically
            grows to accomodate as required.

    Returns: Nothing.

    Exceptions: OutOfMemory.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
    09/06/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
VOID
StructureList::Insert(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));
    DBGASSERT((0 <= index));

    AutoLockCs lock(m_cs);   // Get lock on container.  Will auto-release.

    DBGASSERT((NULL != m_hdsa));

    if (DSA_InsertItem(m_hdsa, index, pvItem) < 0)
    {
        throw CAllocException();
    }
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Replace

    Description: Replaces an item in the Structure list at a given index.
        If the index is beyond the upper bounds of the array, the array
        is extended by one and the item is appended to the list.

    Arguments:
        pvItem - Address of item to replace existing item.

        index - List index where item is to be replaced.  

    Returns:
        TRUE  - Success.
        FALSE - Invalid index or empty container.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
StructureList::Replace(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));
    DBGASSERT((0 <= index));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.

    DBGASSERT((NULL != m_hdsa));

    return DSA_SetItem(m_hdsa, index, pvItem);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Append

    Description: Appends an item to the end of the list.

    Arguments:
        pvItem - Address of item to add to list.

    Returns: Nothing.

    Exceptions: OutOfMemory.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
    09/06/96    Added exception handling.                            BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
VOID
StructureList::Append(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container. Will auto-release.
    //
    // Yes, this is correct.  We're "inserting" an item to append something
    // to the list.  This saves a ( count - 1 ) calculation.
    //
    DBGASSERT((NULL != m_hdsa));
    Insert(pvItem, DSA_GetItemCount(m_hdsa));
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Remove

    Description: Removes an item from the list at a given index.

    Arguments: 
        pvItem - Address of buffer to receive removed item.  Assumes buffer
            is sized properly.

        index - List index where item is to be removed.  All following
            items are shifted to one index lower. 

    Returns:
        TRUE  - Success.
        FALSE - Invalid index or container is empty.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
StructureList::Remove(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));
    DBGASSERT((0 <= index));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.
    DBGASSERT((NULL != m_hdsa));

    if (!DSA_GetItem(m_hdsa, index, pvItem) ||
        !DSA_DeleteItem(m_hdsa, index))
    {
        return FALSE;
    }
    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Retrieve

    Description: Retrieve an item from the list at a given index.
        The item value is merely copied and not removed from the list.

    Arguments:
        pvItem - Address of buffer to receive removed item.  Assumes buffer
            is sized properly.

        index - List index where item is to be retrieved.  

    Returns:
        TRUE  - Success.
        FALSE - Invalid index or empty container.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
StructureList::Retrieve(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));
    DBGASSERT((0 <= index));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.
    DBGASSERT((NULL != m_hdsa));

    if (!DSA_GetItem(m_hdsa, index, pvItem))
    {
        return FALSE;
    }
    return TRUE;
}

    
///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::RemoveLast

    Description: Removes the last item from the list.

    Arguments: 
        pvItem - Address of buffer to receive removed item.  Assumes buffer
            is sized properly.

    Returns:
        TRUE  - Success.
        FALSE - Empty container.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
StructureList::RemoveLast(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.

    DBGASSERT((NULL != m_hdsa));

    if (0 == DSA_GetItemCount(m_hdsa))
        return FALSE;

    Remove(pvItem, DSA_GetItemCount(m_hdsa) - 1);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::RetrieveLast

    Description: Retrieves the last pointer from the list.

    Arguments: 
        pvItem - Address of buffer to receive removed item.  Assumes buffer
            is sized properly.

    Returns:
        TRUE  - Success.
        FALSE - Empty container.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
BOOL
StructureList::RetrieveLast(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.
    DBGASSERT((NULL != m_hdsa));

    if (0 == DSA_GetItemCount(m_hdsa))
        return FALSE;

    Retrieve(pvItem, DSA_GetItemCount(m_hdsa) - 1);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureList::Clear

    Description: Removes all items from the list.

    Arguments: None.

    Returns: Nothing.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/26/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
VOID
StructureList::Clear(
    VOID
    )
{
    AutoLockCs lock(m_cs);  // Get lock on container.  Will auto-release.

    DBGASSERT((NULL != m_hdsa));
    DSA_DeleteAllItems(m_hdsa);
}


///////////////////////////////////////////////////////////////////////////////
/*  Function: StructureListIterator::Advance

    Description: Both the Next() and Prev() iterator functions call this
        one function.  It handles the actual iteration.

    Arguments:
        ppvOut - Address of pointer variable to receive the address of the 
            item at the "current" iterator location.  The iterator is
            advance (or retreated) after the pointer value is copied to the
            destination.

        bForward - TRUE  = Advance toward end of list.
                   FALSE = Advance toward front of list.

    Returns:
        NO_ERROR      - Iterator advanced.  Returned pointer is valid.
        E_FAIL        - Iterator already at begining or end of list.  Returned
                        pointer will be NULL.

    Revision History:

    Date        Description                                          Programmer
    --------    ---------------------------------------------------  ----------
    06/24/96    Initial creation.                                    BrianAu
*/
///////////////////////////////////////////////////////////////////////////////
HRESULT 
StructureListIterator::Advance(
    LPVOID *ppvOut, 
    BOOL bForward
    )
{
    LPVOID pv       = NULL;
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != ppvOut));

    m_pList->Lock();
    if (0 < m_pList->Count() && m_Index != EndOfList)
    {
        //
        // Get address of item at index "m_Index".
        //
        DBGASSERT((NULL != m_pList->m_hdsa));
        pv = DSA_GetItemPtr(m_pList->m_hdsa, m_Index);

        if (bForward)
        {
            //
            // Advance iterator index.
            //
            if ((UINT)(++m_Index) == m_pList->Count())
                m_Index = EndOfList;
        }
        else
        {
            //
            // Retreat iterator index.
            //
            m_Index--;  // Will be -1 (EndOfList) if currently 0.
        }
    }
    else
        hResult = E_FAIL;

    m_pList->ReleaseLock();
    *ppvOut = pv;  // Return pointer value.

    return hResult;
}
