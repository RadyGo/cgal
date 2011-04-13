// ============================================================================
//
// Copyright (c) 2000 The CGAL Consortium
//
// This software and related documentation is part of an INTERNAL release
// of the Computational Geometry Algorithms Library (CGAL). It is not
// intended for general use.
//
// ----------------------------------------------------------------------------
//
// release       : $CGAL_Revision $
// release_date  : $CGAL_Date $
//
// file          : include/CGAL/partition_is_valid_2.h
// package       : $CGAL_Package: Partition_2 $
// maintainer    : Susan Hert <hert@mpi-sb.mpg.de>
// chapter       : Planar Polygon Partitioning
//
// revision      : $Revision$
// revision_date : $Date$
//
// author(s)     : Susan Hert <hert@mpi-sb.mpg.de>
//
// coordinator   : MPI (Susan Hert <hert@mpi-sb.mpg.de>)
//
// implementation: Polygon partitioning validity checking functions.
// ============================================================================

#ifndef CGAL_PARTITION_IS_VALID_2_H
#define CGAL_PARTITION_IS_VALID_2_H

#include <list>
#include <utility>
#include <iterator>
#include <CGAL/partition_assertions.h>
#include <CGAL/Partitioned_polygon_2.h>
#include <CGAL/Partition_vertex_map.h>
#include <CGAL/ch_selected_extreme_points_2.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/Partition_is_valid_traits_2.h>
#include <CGAL/Polygon_2.h>

// NOTE:  this could possibly be checked using a planar map overlay, but
//        then the traits class would have to require lots of other things
//        and you have to do many overlaps, not just one so it is
//        less efficient.

namespace CGAL {

template <class Circulator1, class Circulator2>
bool 
polygons_w_steiner_are_equal(Circulator1 orig_first, Circulator2 new_first)
{
   typedef typename Circulator1::value_type                Point_2;

   Circulator1 orig_circ;
   Circulator2 new_circ;

   // find the first (original) vertex in the list of vertices
   for (new_circ = new_first; 
        *new_circ != *orig_first && ++new_circ != new_first;) 
   {}

   if (new_circ == new_first) 
   {
#ifdef CGAL_PARTITION_CHECK_DEBUG
      std::cout << "first vertex " << *orig_first << " not found " 
                << std::endl;
#endif // CGAL_PARTITION_CHECK_DEBUG
      return false;
   }

   // first becomes the first one you found; now look for the others
   new_first = new_circ;
   orig_circ = orig_first;
   Point_2 prev_pt = *new_first;
  
   // keep going until you find all the original vertices, or come back
   // to the first new vertex
   do
   {
      if (*new_circ == *orig_circ)  // points correspond, so move both
      {
         prev_pt = *new_circ;
         new_circ++;
         orig_circ++;
      }
      else // points don't correspond
      {
         if (!collinear(prev_pt, *new_circ, *orig_circ))
         {
#ifdef CGAL_PARTITION_CHECK_DEBUG
           std::cout << *new_circ << " is not collinear with " << prev_pt 
                     << " and " << *orig_circ << std::endl;
#endif
           return false;
         }
         if (!collinear_are_ordered_along_line(prev_pt, *new_circ, *orig_circ))
         {
#ifdef CGAL_PARTITION_CHECK_DEBUG
           std::cout << *new_circ << " doesn't belong betweene " << prev_pt 
                     << " and " << *orig_circ << std::endl;
#endif
           return false;
         }
         prev_pt = *new_circ;
         new_circ++;
      }
   }
   while (orig_circ != orig_first && new_circ != new_first);

   // if they didn't both come back to the beginning, then something is wrong
   return (orig_circ == orig_first && new_circ == new_first);
}

template <class Circulator1, class Circulator2>
bool 
polygons_are_equal(Circulator1 orig_first, Circulator2 new_first)
{
   Circulator1 orig_circ = orig_first;
   Circulator2 new_circ;

   // find the first (original) vertex in the list of vertices
   for (new_circ = new_first; 
        *new_circ != *orig_first && ++new_circ != new_first;) 
   {}

   new_first = new_circ;
   // now look through both lists until you find a vertex that is not
   // the same or you reach the end of the vertices
   do 
   {
#ifdef CGAL_PARTITION_CHECK_DEBUG
      std::cout << *new_first << " is in the right place " << std::endl;
#endif
      orig_circ++; new_circ++;
   }
   while (*orig_circ == *new_circ &&
          orig_circ != orig_first && new_circ != new_first);

   return (orig_circ == orig_first && new_circ == new_first);
}


template<class InputIterator, class ForwardIterator, class Traits> 
bool
partition_is_valid_2 (InputIterator point_first, InputIterator point_last,
                      ForwardIterator poly_first, ForwardIterator poly_last,
                      const Traits& traits) 
{
   if (poly_first == poly_last)  return (point_first == point_last);

   typedef typename Traits::Polygon_2::Vertex_iterator   
                                                       Poly_vtx_iterator;
   typedef typename Traits::Point_2                    Point_2;
   typedef Partition_vertex_map<Traits>                P_Vertex_map;
   typedef typename Traits::Is_valid                   Is_valid;

   Poly_vtx_iterator vtx_begin, vtx_end;

   Is_valid is_valid = traits.is_valid_object(traits);

   std::list<Point_2>    orig_poly;
   for (;point_first != point_last; point_first++)
      orig_poly.push_back(*point_first);

   CGAL_partition_precondition(orientation_2(orig_poly.begin(),orig_poly.end(),
                                             traits) == COUNTERCLOCKWISE);

   P_Vertex_map  output_vertex_set(poly_first, poly_last);

   if (output_vertex_set.polygons_overlap()) return false;

   int poly_num = 0;
   for (; poly_first != poly_last; poly_first++, poly_num++)
   {
      vtx_begin = (*poly_first).vertices_begin();
      vtx_end = (*poly_first).vertices_end();
#ifdef CGAL_PARTITION_CHECK_DEBUG
         std::cout << "Polygon " << poly_num << " is " << std::endl;
         std::cout << *poly_first << std::endl;
#endif
      CGAL_partition_assertion (
           orientation_2(vtx_begin, vtx_end, traits) == COUNTERCLOCKWISE);
      if (!is_valid(vtx_begin, vtx_end)) 
      {
#ifdef CGAL_PARTITION_CHECK_DEBUG
         std::cout << "It does NOT have the tested property." << std::endl;
#endif
         return false;
      }
   }

   std::list<Point_2>  union_polygon;
   output_vertex_set.union_vertices(std::back_inserter(union_polygon));

#ifdef CGAL_PARTITION_CHECK_DEBUG
   typename std::list<Point_2>::iterator  poly_iterator;
   std::cout << "union polygon is " << std::endl;
   for (poly_iterator = union_polygon.begin(); 
        poly_iterator != union_polygon.end(); poly_iterator++)
   {
      std::cout << *poly_iterator << " ";
   }
   std::cout << std::endl;
#endif // CGAL_PARTITION_CHECK_DEBUG

   typedef typename std::list<Point_2>::iterator     I;
   typedef Circulator_from_iterator<I>      Circulator;

   Circulator   orig_poly_circ(orig_poly.begin(), orig_poly.end());
   Circulator   union_poly_circ(union_polygon.begin(), union_polygon.end());
   if (orig_poly.size() == union_polygon.size())
     return polygons_are_equal(orig_poly_circ, union_poly_circ);
   else
     return polygons_w_steiner_are_equal(orig_poly_circ, union_poly_circ);
}

template<class InputIterator, class FowardIterator>
bool
partition_is_valid_2 (InputIterator point_first, InputIterator point_last,
                      FowardIterator poly_first, FowardIterator poly_last)
{
   typedef typename std::iterator_traits<InputIterator>::value_type   Point_2;
   return CGAL_partition_is_valid_2(point_first, point_last,
                                    poly_first, poly_last,
                                    reinterpret_cast<Point_2*>(0));
}

template<class InputIterator, class ForwardIterator, class R>
bool
CGAL_partition_is_valid_2 (InputIterator point_first, 
                           InputIterator point_last,
                           ForwardIterator poly_first, 
                           ForwardIterator poly_last,
                           Point_2<R>*)
{
   typedef Partition_traits_2<R>                   Traits;
   typedef Is_vacuously_valid<Traits>              Is_valid;

   Partition_is_valid_traits_2<Traits, Is_valid>   validity_traits;

   return partition_is_valid_2(point_first, point_last, poly_first, poly_last,
                               validity_traits);
}


template<class InputIterator, class ForwardIterator, class Traits>
bool 
convex_partition_is_valid_2(InputIterator point_first,
                            InputIterator point_last,
                            ForwardIterator poly_first,
                            ForwardIterator poly_last,
                            const Traits& )
{
   typedef typename Traits::Is_convex_2                 Is_convex_2;
   Partition_is_valid_traits_2<Traits, Is_convex_2>     validity_traits;

   return partition_is_valid_2(point_first, point_last, poly_first, poly_last,
                               validity_traits);
}

template<class InputIterator, class ForwardIterator>
bool 
convex_partition_is_valid_2(InputIterator point_first,
                            InputIterator point_last,
                            ForwardIterator poly_first,
                            ForwardIterator poly_last)
{
   typedef typename std::iterator_traits<InputIterator>::value_type   Point_2;
   return CGAL_convex_partition_is_valid_2(point_first, point_last, 
                                           poly_first, poly_last, 
                                           reinterpret_cast<Point_2*>(0));
}

template<class InputIterator, class ForwardIterator, class R>
bool 
CGAL_convex_partition_is_valid_2(InputIterator point_first,
                                 InputIterator point_last,
                                 ForwardIterator poly_first,
                                 ForwardIterator poly_last,
                                 Point_2<R>*)
{
   return convex_partition_is_valid_2(point_first, point_last, 
                                      poly_first, poly_last, 
                                      Partition_traits_2<R>());
}

template<class InputIterator, class ForwardIterator, class Traits>
bool 
y_monotone_partition_is_valid_2(InputIterator point_first,
                                InputIterator point_last,
                                ForwardIterator poly_first,
                                ForwardIterator poly_last,
                                const Traits& )
{
   typedef typename Traits::Is_y_monotone_2                Is_y_monotone_2;

   Partition_is_valid_traits_2<Traits, Is_y_monotone_2>    validity_traits;

   return partition_is_valid_2(point_first, point_last, poly_first, poly_last,
                               validity_traits);
}

template<class InputIterator, class ForwardIterator>
bool 
y_monotone_partition_is_valid_2(InputIterator point_first,
                                InputIterator point_last,
                                ForwardIterator poly_first,
                                ForwardIterator poly_last)
{
   typedef typename std::iterator_traits<InputIterator>::value_type   Point_2;
   return CGAL_y_monotone_partition_is_valid_2(point_first, point_last, 
                                               poly_first, poly_last,
                                               reinterpret_cast<Point_2*>(0));
}

template<class InputIterator, class ForwardIterator, class R>
bool
CGAL_y_monotone_partition_is_valid_2(InputIterator point_first,
                                     InputIterator point_last,
                                     ForwardIterator poly_first,
                                     ForwardIterator poly_last,
                                     Point_2<R>*)
{
   return y_monotone_partition_is_valid_2(point_first, point_last, 
                                          poly_first, poly_last, 
                                          Partition_traits_2<R>());
}

}

#endif // CGAL_PARTITION_IS_VALID_2_H