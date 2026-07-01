//BTreePage.h

/*************************
#ifndef BTPage_H
#define BTPage_H
***************************/
#ifndef CBTreePage_H
#define CBTreePage_H

#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

#include "../types.h"
#include "traits.h"

template <typename Traits>
class BTree;

enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

template <typename Traits>
class CBTreePage
// this is the in-memory version of the CBTreePage
{
       friend class BTree<Traits>;

public:
       using value_type = typename Traits::value_type;
       using ref_type = typename Traits::ref_type;
       using node_type = typename Traits::node_type;
       using compare_type = typename Traits::compare_type;
       using page_type = CBTreePage<Traits>;
       using ObjectInfo = node_type;

       CBTreePage(std::size_t maxKeys, Bool unique = true);
       virtual ~CBTreePage();

       bt_ErrorCode    Insert (const value_type &key, const ref_type& ref);
       bt_ErrorCode    Remove (const value_type &key, const ref_type& ref);
       Bool            Search (const value_type &key, ref_type &ref);
       void            Print  (std::ostream &os);

       template <typename Func, typename... Args>
       void ForEach(Func&& func, std::size_t level, Args&&... args)
       {
              FirstThat(
                     [&](node_type& info, std::size_t currentLevel) -> Bool
                     {
                            std::invoke(func, info, currentLevel, args...);
                            return false;
                     },
                     level);
       }

       template <typename Predicate, typename... Args>
       node_type* FirstThat(Predicate&& pred, std::size_t level, Args&&... args)
       {
              for( std::size_t i = 0 ; i < m_KeyCount ; i++)
              {
                     if( m_SubPages[i] )
                     {
                            node_type* pTmp = m_SubPages[i]->FirstThat(
                                   std::forward<Predicate>(pred),
                                   level+1,
                                   std::forward<Args>(args)...);
                            if( pTmp )
                                   return pTmp;
                     }
                     if( std::invoke(pred, m_Keys[i], level, std::forward<Args>(args)...) )
                            return &m_Keys[i];
              }
              if( m_SubPages[m_KeyCount] )
              {
                     node_type* pTmp = m_SubPages[m_KeyCount]->FirstThat(
                            std::forward<Predicate>(pred),
                            level+1,
                            std::forward<Args>(args)...);
                     if( pTmp )
                            return pTmp;
              }
              return nullptr;
       }

protected:
       std::size_t  m_MinKeys; // minimum number of keys in a node
       std::size_t  m_MaxKeys, // maximum number of keys in a node
                    m_MaxKeysForChilds; // just to distinguish the root
       Bool m_Unique;
       Bool m_isRoot;
       std::vector<ObjectInfo> m_Keys;
       std::vector<page_type *>   m_SubPages;
       std::size_t  m_KeyCount;
       void  Create();
       void  Reset ();
       void  Destroy () {   Reset(); delete this;}
       void  clear ();

       Bool  Redistribute1   (std::size_t &pos);
       Bool  Redistribute2   (std::size_t pos);
       void  RedistributeR2L (std::size_t pos);
       void  RedistributeL2R (std::size_t pos);

       Bool    TreatUnderflow  (std::size_t &pos)
       {       return Redistribute1(pos) || Redistribute2(pos);}

       bt_ErrorCode    Merge  (std::size_t pos);
       bt_ErrorCode    MergeRoot ();
       void  SplitChild (std::size_t pos);

       ObjectInfo &GetFirstObjectInfo();

       Bool Overflow() const  { return m_KeyCount > m_MaxKeys; }
       Bool Underflow() const { return m_KeyCount < MinNumberOfKeys(); }
       Bool IsFull() const    { return m_KeyCount >= m_MaxKeys; }
       std::size_t  MinNumberOfKeys() const  { return 2*m_MaxKeys/3; }
       std::size_t  GetFreeCells() const  { return m_MaxKeys - m_KeyCount; }
       std::size_t& NumberOfKeys()  { return m_KeyCount; }
       std::size_t  GetNumberOfKeys() const  { return m_KeyCount; }
       Bool IsRoot() const  { return m_MaxKeysForChilds != m_MaxKeys; }
       void SetMaxKeysForChilds(std::size_t orderforchilds)
       {
               m_MaxKeysForChilds = orderforchilds;
       }

       std::size_t GetFreeCellsOnLeft(std::size_t pos);
       std::size_t GetFreeCellsOnRight(std::size_t pos);

private:
       Bool SplitRoot();
       void SplitPageInto3(std::vector<ObjectInfo>   & tmpKeys,
                           std::vector<page_type *>  & tmpSubPages,
                           page_type                 *& pChild1,
                           page_type                 *& pChild2,
                           page_type                 *& pChild3,
                           ObjectInfo                & oi1,
                           ObjectInfo                & oi2);
       void MovePage(page_type *pChildPage,
                     std::vector<ObjectInfo> & tmpKeys,
                     std::vector<page_type *> & tmpSubPages);
};

// Si no lo encuentra, deberia decirme:
// cual es la posicion donde deberia estar
template <typename Container, typename ObjType, typename Compare>
std::size_t binary_search(Container& container, std::size_t first, std::size_t last, const ObjType &object, Compare compare)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               std::size_t mid = (first+last)/2;
               ObjType current = static_cast<ObjType>(container[mid]);
               if( !compare(object, current) && !compare(current, object) )
                       return mid;
               if( compare(current, object) )
                       first = mid+1;
               else
                       last  = mid;
       }
       ObjType current = static_cast<ObjType>(container[first]);
       if( !compare(current, object) )
               return first;
       return last;
}

template <typename Container, typename ObjType>
void insert_at(Container& container, const ObjType &object, std::size_t pos)
{
       std::size_t size = container.size();
       for(std::size_t i = size - 1 ; i > pos ; --i)
               container[i] = container[i-1];
       container[pos] =  object;
}

template <typename Container>
void remove(Container& container, std::size_t pos)
{
       std::size_t size = container.size();
       for(std::size_t i = pos+1 ; i < size ; i++)
               container[i-1] = container[i];
}

template <typename Traits>
CBTreePage<Traits>::CBTreePage(std::size_t maxKeys, Bool unique)
                                       : m_MaxKeys(maxKeys), m_Unique(unique), m_isRoot(false), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Traits>
CBTreePage<Traits>::~CBTreePage()
{
       Reset();
}

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Insert(const value_type& key, const ref_type& ref)
{
       compare_type compare;
       std::size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       bt_ErrorCode error = bt_ok;

       if( pos < m_KeyCount &&
           !compare(key, m_Keys[pos].key) &&
           !compare(m_Keys[pos].key, key) &&
           m_Unique)
               return bt_duplicate; // this key is duplicate

       if( !m_SubPages[pos] ) // this is a leave
       {
               ::insert_at(m_Keys, ObjectInfo(key, ref), pos);
               NumberOfKeys()++;
               if( Overflow() )
                       return bt_overflow;
               return bt_ok;
       }
       else
       {
               // recursive insertion
               error = m_SubPages[pos]->Insert(key, ref);
               if( error == bt_overflow )
               {
                       if( !Redistribute1(pos) )
                               SplitChild(pos);
                       if( Overflow() )          // Propagate overflow
                               return bt_overflow;
                       return bt_ok;
               }
       }

       // Nunca va a entrar a este If porque esta situacion
       // debe haber sido tratada en el if anterior
       if( Overflow() ) // node overflow
               return bt_overflow;
       return bt_ok;
}

template <typename Traits>
Bool CBTreePage<Traits>::Redistribute1(std::size_t &pos)
{
       if( m_SubPages[pos]->Underflow() )
       {       // nkol = Number of keys on left brother, nkor = Number of keys on right brother
               std::size_t nkol = 0,
                           nkor = 0;
               // is this the first element or there are more elements on right brother
               if( pos > 0 )
                       nkol = m_SubPages[pos-1]->NumberOfKeys();
               if( pos < NumberOfKeys() )
                       nkor = m_SubPages[pos+1]->NumberOfKeys();

               if( nkol > nkor )
                       if( m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys() )
                               RedistributeL2R(pos-1); // bring elements from left brother
                       else
                               if( pos == NumberOfKeys() )
                                       return (--pos, false);
                               else
                                       return false;
               else //nkol < nkor )
                       if( m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys() )
                               RedistributeR2L(pos+1); // bring elements from right brother
                       else
                               if( pos == 0 )
                                       return (++pos, false);
                               else
                                       return false;
       }
       else // it is due to overflow
       {
               std::size_t fcol = GetFreeCellsOnLeft(pos),   // Free Cells On Left
                           fcor = GetFreeCellsOnRight(pos);  // Free Cells On Right

               if( !fcol && !fcor && m_SubPages[pos]->IsFull() )
                       return false;
               if( fcol > fcor ) // There is more space on left
                       RedistributeR2L(pos);
               else
                       RedistributeL2R(pos);

       }
       return true;
}

// Redistribute2 function
// it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
// if it fails the only way is merge !
template <typename Traits>
Bool CBTreePage<Traits>::Redistribute2(std::size_t pos)
{
       assert( pos > 0 && pos < NumberOfKeys()  );
       assert( m_SubPages[pos-1] != nullptr && m_SubPages[pos] != nullptr && m_SubPages[pos+1] != nullptr );
       assert( m_SubPages[pos-1]->Underflow() ||
                       m_SubPages[ pos ]->Underflow() ||
                       m_SubPages[pos+1]->Underflow() );

       if( m_SubPages[pos-1]->Underflow() )
       {       // Rotate R2L
               RedistributeR2L(pos+1);
               RedistributeR2L(pos);
               if( m_SubPages[pos-1]->Underflow() )
                       return false;
       }
       else if( m_SubPages[pos+1]->Underflow() )
       {       // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeL2R(pos);
               if( m_SubPages[pos+1]->Underflow() )
                       return false;
       }
       else // The problem is exactly at pos !
       {
               // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeR2L(pos+1);
               if( m_SubPages[pos]->Underflow() )
                       return false;
       }
       return true;
}

template <typename Traits>
void CBTreePage<Traits>::RedistributeR2L(std::size_t pos)
{
       page_type  *pSource = m_SubPages[ pos ],
                  *pTarget = m_SubPages[pos-1];

       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
             pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-left page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos-1], pTarget->NumberOfKeys()++);
               // Move the pointer leftest pointer to the rightest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

               // Move the leftest element to the root
               m_Keys[pos-1] = pSource->m_Keys[0];

               // Remove the leftest element from rigth page
               ::remove(pSource->m_Keys    , 0);
               ::remove(pSource->m_SubPages, 0);
               pSource->NumberOfKeys()--;
       }
}

template <typename Traits>
void CBTreePage<Traits>::RedistributeL2R(std::size_t pos)
{
       page_type  *pSource = m_SubPages[pos],
                  *pTarget = m_SubPages[pos+1];
       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-RIGHT page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
               // Move the pointer rightest pointer to the leftest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
               pTarget->NumberOfKeys()++;

               // Move the rightest element to the root
               m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys()-1];

               // Remove the leftest element from rigth page
               // it is not necessary erase because m_KeyCount controls
               pSource->NumberOfKeys()--;
       }
}

template <typename Traits>
void CBTreePage<Traits>::SplitChild(std::size_t pos)
{
       // FIRST: deciding the second page to split
       page_type  *pChild1 = nullptr, *pChild2 = nullptr;
       if( pos > 0 )                                   // is left page full ?
               if( m_SubPages[pos-1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos-1];
                       pChild2 = m_SubPages[pos--];
               }
       if( pos < GetNumberOfKeys() )   // is right page full ?
               if( m_SubPages[pos+1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos];
                       pChild2 = m_SubPages[pos+1];
               }

       // SECOND: copy both pages to a temporal one
       // Create two tmp vector
       std::vector<ObjectInfo> tmpKeys;
       std::vector<page_type *> tmpSubPages;

       // Prepara el vectpor unificado de las 2 paginas a ser divididas en 3
       // copy from left child
       MovePage(pChild1, tmpKeys, tmpSubPages);
       // copy a key from parent
       tmpKeys    .push_back(m_Keys[pos]);

       // copy from right child
       MovePage(pChild2, tmpKeys, tmpSubPages);

       page_type *pChild3 = nullptr;
       ObjectInfo oi1, oi2;
       SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

       // copy the first element to the root
       m_Keys    [pos] = oi1;
       m_SubPages[pos] = pChild1;

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
}

template <typename Traits>
void CBTreePage<Traits>::SplitPageInto3(std::vector<ObjectInfo>& tmpKeys,
                                        std::vector<page_type *> & tmpSubPages,
                                        page_type*               & pChild1,
                                        page_type*               & pChild2,
                                        page_type*               & pChild3,
                                        ObjectInfo               & oi1,
                                        ObjectInfo               & oi2)
{
       assert(tmpKeys.size() >= 8);
       assert(tmpSubPages.size() >= 9);
       if( !pChild1 )
               pChild1 = new page_type(m_MaxKeysForChilds, m_Unique);

       // Split tmpKeys page into 3 pages
       // copy 1/3 elements to the first child
       pChild1->clear();
       std::size_t nKeys = (tmpKeys.size()-2)/3;
       std::size_t i = 0;
       for( ; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       // first element to go up !
       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new page_type(m_MaxKeysForChilds, m_Unique);
       pChild2->clear();
       // copy 1/3 to the second child
       nKeys += (tmpKeys.size()-2)/3 + 1;
       std::size_t j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];

       // copy the second element to the root
       oi2 = tmpKeys[i++];

       // copy 1/3 to the third child
       if( !pChild3 )
               pChild3 = new page_type(m_MaxKeysForChilds, m_Unique);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
}

template <typename Traits>
Bool CBTreePage<Traits>::SplitRoot()
{
       page_type  *pChild1 = nullptr, *pChild2 = nullptr, *pChild3 = nullptr;
       ObjectInfo oi1, oi2;
       SplitPageInto3( m_Keys,m_SubPages,pChild1, pChild2, pChild3, oi1, oi2);
       clear();

       // copy the first element to the root
       m_Keys    [0] = oi1;
       m_SubPages[0] = pChild1;
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       return true;
}

template <typename Traits>
Bool CBTreePage<Traits>::Search(const value_type &key, ref_type &ref)
{
       compare_type compare;
       std::size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       if( pos >= m_KeyCount ){
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ref);
               else
                       return false;
       }
       if( !compare(key, m_Keys[pos].key) && !compare(m_Keys[pos].key, key) )
       {
               ref = m_Keys[pos].ref;
               m_Keys[pos].use_counter++;
               return true;
       }
       if( compare(key, m_Keys[pos].key) )
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ref);
       return false;
}

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Remove(const value_type &key, const ref_type& ref)
{
       bt_ErrorCode error = bt_ok;
       compare_type compare;
       std::size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       if( pos < NumberOfKeys() &&
           !compare(key, m_Keys[pos].key) &&
           !compare(m_Keys[pos].key, key) /*&& m_Keys[pos].ref == ref*/)
       {
               // This is a leave: First
               if( !m_SubPages[pos+1] )  // This is a leave ? FIRST CASE !
               {
                       ::remove(m_Keys, pos);
                       NumberOfKeys()--;
                       if( Underflow() )
                               return bt_underflow;
                       return bt_ok;
               }

               // We FOUND IT BUT it is NOT a leave ? SECOND CASE !
               {
                       // Get the first element from right branch
                       ObjectInfo &rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
                       // change with a leave
                       std::swap(m_Keys[pos], rFirstFromRight);
                       // Remove it from this leave

                       //Print(cout);
                       error = m_SubPages[++pos]->Remove(key, ref);
               }
       }
       else if( pos == NumberOfKeys() ) // it is not here, go by the last branch
       {
               if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(key, ref);
               else
                       return bt_nofound;
       }
       else if( !compare(m_Keys[pos].key, key) ){ // = is because identical keys are inserted on left (see Insert)
               if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(key, ref);
               else
                       return bt_nofound;
       }
       if( error == bt_underflow ){
               // THIRD CASE: After removing the element we have an underflow
               //Print(cout);
               if( TreatUnderflow(pos) )
                       return bt_ok;
               // FOURTH CASE: it was not possible to redistribute -> Merge
               if( IsRoot() && NumberOfKeys() == 2 )
                       return MergeRoot();
               return Merge(pos);
       }
       if( error == bt_nofound )
               return bt_nofound;
       return bt_ok;
}


template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Merge(std::size_t pos)
{
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                m_SubPages[ pos ]->NumberOfKeys() +
                m_SubPages[pos+1]->NumberOfKeys() ==
                3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       // FIRST: Put all the elements into a vector
       std::vector<ObjectInfo> tmpKeys;
       std::vector<page_type *> tmpSubPages;

       page_type  *pChild1 = m_SubPages[pos-1],
                  *pChild2 = m_SubPages[ pos ],
                  *pChild3 = m_SubPages[pos+1];
       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);
       pChild3->Destroy();

       // Move 1/2 elements to pChild1
       std::size_t nKeys = pChild1->GetFreeCells();
       std::size_t i = 0;
       for( ; i < nKeys ; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();
       std::size_t j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       m_SubPages[ pos ]          = pChild2;

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::MergeRoot()
{
       std::size_t pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       page_type  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
       std::size_t nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

       // FIRST: Put all the elements into a vector
       std::vector<ObjectInfo> tmpKeys;
       std::vector<page_type *> tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);

       clear();
       std::size_t i = 0;
       for( ; i < nKeys ; i++ ){
               m_Keys    [i] = tmpKeys    [i];
               m_SubPages[i] = tmpSubPages[i];
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];

       //Print(cout);
       pChild1->Destroy();
       pChild2->Destroy();
       pChild3->Destroy();

       return bt_rootmerged;
}

template <typename Traits>
typename CBTreePage<Traits>::ObjectInfo &
CBTreePage<Traits>::GetFirstObjectInfo()
{
       if( m_SubPages[0] )
               return m_SubPages[0]->GetFirstObjectInfo();
       return m_Keys[0];
}

template <typename Traits>
void CBTreePage<Traits>::Print(std::ostream & os)
{
       ForEach(
              [](const ObjectInfo& info, std::size_t level, std::ostream& out)
              {
                     for( std::size_t i = 0; i < level ; i++)
                            out << "\t";
                     out << info << "\n";
              },
              0,
              os);
}

template <typename Traits>
void CBTreePage<Traits>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys+1);
       m_SubPages.resize(m_MaxKeys+2, nullptr);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename Traits>
void CBTreePage<Traits>::Reset()
{
       for( std::size_t i = 0 ; i < m_KeyCount ; i++ )
               delete m_SubPages[i];
       clear();
}

template <typename Traits>
void CBTreePage<Traits>::clear()
{
       //m_Keys.clear();
       //m_SubPages.clear();
       m_KeyCount = 0;
}

template <typename Traits>
CBTreePage<Traits> * CreateBTreeNode (std::size_t maxKeys, Bool unique)
{
       return new CBTreePage<Traits> (maxKeys, unique);
}

template <typename Traits>
void CBTreePage<Traits>::MovePage(page_type *pChildPage,
                                  std::vector<ObjectInfo> &tmpKeys,
                                  std::vector<page_type *> &tmpSubPages)
{
       std::size_t nKeys = pChildPage->GetNumberOfKeys();
       std::size_t i = 0;
       for( ; i < nKeys; i++ )
       {
               tmpKeys    .push_back(pChildPage->m_Keys[i]);
               tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       pChildPage->clear();
}

template <typename Traits>
std::size_t CBTreePage<Traits>::GetFreeCellsOnLeft(std::size_t pos)
{
       if( pos > 0 )                                   // there is some page on left ?
               return m_SubPages[pos-1]->GetFreeCells();
       return 0;
}

template <typename Traits>
std::size_t CBTreePage<Traits>::GetFreeCellsOnRight(std::size_t pos)
{
       if( pos < GetNumberOfKeys() )   // there is some page on right ?
               return m_SubPages[pos+1]->GetFreeCells();
       return 0;
}

#endif
