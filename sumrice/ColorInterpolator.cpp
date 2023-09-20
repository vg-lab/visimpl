//
// Created by gaeqs on 6/22/22.
//

#include "ColorInterpolator.h"
#include "Utils.h"


glm::vec4 ColorInterpolator::getValue( float percentage )
{
  if ( data.empty( )) return glm::vec4( 0 );
  auto first = data[ 0 ];
  if ( first.first >= percentage ) return first.second;

  auto& last = data.back();
  if ( percentage >= last.first ) return last.second;

  auto next = visimpl::lower_bound(
    data.cbegin( ) , data.cend( ) , percentage ,
    [ ](
      const std::pair< float , glm::vec4 >& e ,
      float v )
    {
      return e.first < v;
    } );

  auto value = next - 1;
  float n = ( percentage - value->first ) / ( next->first - value->first );
  return glm::mix( value->second , next->second , n );
}

void ColorInterpolator::insert( float percentage , glm::vec4 value )
{
  auto position = visimpl::lower_bound(
    data.begin( ) , data.end( ) , percentage ,
    [ ](
      const std::pair< float , glm::vec4 >& e ,
      float v )
    {
      return e.first < v;
    } );
  data.insert( position , std::make_pair( percentage , value ));
}