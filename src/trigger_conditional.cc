/*
 * Copyright (C) 2002-5 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include "error.h"
#include "event_chain.h"
#include "trigger.h"
#include "trigger_conditional.h"

/*
 * TriggerConditional_Factory
 */

/*
 * Creates trigger conditionals, throws SyntaxError on error
 * This basically only converts the infix into postfix notation
 */
TriggerConditional* TriggerConditional_Factory::create_from_infix( EventChain* evchain, const std::vector<Token>& vec ) {

   std::vector<Token> tempstack;
   std::vector<Token> postfix;

   for( uint i = 0; i < vec.size(); i++) {
      if( vec[i].token == LPAREN ) {
         // Left parenthesis, push to stack
         tempstack.push_back( vec[i] );
      } else if ( vec[i].token == RPAREN ) {
         // Right parenthesis, walk stack
         // append everything to our postfix notation
         if( !tempstack.size()) {
            // Mismatched parathesis
            ALIVE();
            log( "Missmatched parenthesis!\n");
            throw SyntaxError();
         }
         while( tempstack.back().token != LPAREN) {
            if( !tempstack.size()) { // Mismatched parathesis
               ALIVE();
               log( "Missmatched parenthesis!\n");
               throw SyntaxError();
            }
            postfix.push_back( tempstack.back() );
            tempstack.pop_back();
         }
         tempstack.pop_back(); // Pop the last left paranthesis
      } else if( vec[i].token == TRIGGER ) {
         postfix.push_back( vec[i] );
      } else if( vec[i].token >= OPERATOR ) {
         bool pushed = false;
         while( !pushed ) {
            if( !tempstack.size() ) {
               // If stack empty, push operator
               tempstack.push_back( vec[i] );
               pushed = true;
            } else if( vec[i].token/10 > tempstack.back().token/10 )  {
               // If precendence is higher, push operator
               tempstack.push_back( vec[i] );
               pushed = true;
            } else {
               // Pop an operator
               assert( tempstack.back().token >= OPERATOR);
               postfix.push_back( tempstack.back() );
               tempstack.pop_back();
            }
         }
      } else {
         // Decoding shot stupid things
         ALIVE();
         log( "Decoding shot supid things!\n");
         throw SyntaxError();
      }
   }

   // Unload all operators which are left on stack
   while( tempstack.size() ) {
      if( tempstack.back().token == LPAREN) {
         // Unmatched parentesis
         ALIVE();
         log( "Unmatched parenthesis!\n");
         throw SyntaxError();
      }
      postfix.push_back( tempstack.back() );
      tempstack.pop_back();
   }

   return create_from_postfix(evchain, postfix);
}

/*
 * This effectivly creates a trigger conditional
 * out of a token list.
 *
 * Note that this function expects valid syntax.
 */
TriggerConditional* TriggerConditional_Factory::create_from_postfix( EventChain* evchain, const std::vector<Token>& vec ) {
   std::vector< TriggerConditional* > stk;

   for( uint i = 0; i < vec.size(); i++) {
      if( vec[i].token == TRIGGER) {
         Trigger* trig = static_cast<Trigger*>( vec[i].data );
         trig->reference( evchain );
         stk.push_back( new TriggerConditional_Const( trig ));
      } else if( vec[i].token >= XOR && vec[i].token <= NOT) {
         // Operator
         switch( vec[i].token ) {
            case AND:
            case OR:
            case XOR:
            {
               TriggerConditional* r = stk.back(); stk.pop_back();
               TriggerConditional* l = stk.back(); stk.pop_back();
               if( vec[i].token == AND )
                  stk.push_back( new TriggerAND( r, l ) );
               else if( vec[i].token == OR )
                  stk.push_back( new TriggerOR( r, l ) );
               else if( vec[i].token == XOR )
                  stk.push_back( new TriggerXOR( r, l ) );
            }
            break;

            case NOT:
            {
               TriggerConditional* cond = stk.back(); stk.pop_back();
               stk.push_back( new TriggerNOT( cond ));
            }
            break;

            default:
            // This shouldn't happen
            assert( 0 );
            break;
         }
      } else {
         // Baaad thing
         throw SyntaxError();
      }
   }

   if( stk.size() != 1)
      throw SyntaxError();

   return stk.back();
}

/*
 * Unreference all triggers found
 */
void TriggerConditional::unreference_triggers( EventChain* ev ) {
   std::vector< TriggerConditional_Factory::Token >* vec = get_infix_tokenlist();

   for( uint i = 0; i < vec->size(); i++ ) {
      if( (*vec)[i].token == TriggerConditional_Factory::TRIGGER ) {
         Trigger* trig = static_cast<Trigger*>((*vec)[i].data);
         trig->unreference( ev );
      }
   }
}

/*
 * Reset all triggers found
 */
void TriggerConditional::reset_triggers( Game* g ) {
   std::vector< TriggerConditional_Factory::Token >* vec = get_infix_tokenlist();

   for( uint i = 0; i < vec->size(); i++ ) {
      if( (*vec)[i].token == TriggerConditional_Factory::TRIGGER ) {
         Trigger* trig = static_cast<Trigger*>((*vec)[i].data);
         trig->reset_trigger( g );
      }
   }
}

/*
 * BASE CLASSES
 */

/*
 * TriggerConditional_OneArg
 */
TriggerConditional_OneArg::TriggerConditional_OneArg(TriggerConditional* cond) {
   m_conditional = cond;
}
TriggerConditional_OneArg::~TriggerConditional_OneArg( void ) {
   if( m_conditional )
      delete m_conditional;
}
bool TriggerConditional_OneArg::eval( Game* g) {
   assert( m_conditional );

   bool result = m_conditional->eval( g );

   return do_eval( result );
}
std::vector< TriggerConditional_Factory::Token >* TriggerConditional_OneArg::get_infix_tokenlist( void ) {
   std::vector< TriggerConditional_Factory::Token >* retval = new std::vector< TriggerConditional_Factory::Token >;

   TriggerConditional_Factory::Token tok;
   tok.data = 0;

   // (
   tok.token = TriggerConditional_Factory::LPAREN; retval->push_back( tok );

   // Our token name
   tok.token = get_token();  retval->push_back( tok );

   // Now add our conditionals tokenlist
   std::vector< TriggerConditional_Factory::Token >* l = m_conditional->get_infix_tokenlist();
   for( uint i = 0; i < l->size(); i++)
      retval->push_back( (*l)[i] );
   delete l;

   // )
   tok.token = TriggerConditional_Factory::RPAREN; retval->push_back( tok );

   return retval;
}

/*
 * TriggerConditional_Const
 */
TriggerConditional_Const::TriggerConditional_Const(Trigger* trig) {
   m_trigger = trig;
}
bool TriggerConditional_Const::eval( Game* g ) {
   m_trigger->check_set_conditions( g );
   return m_trigger->is_set();
}
std::vector< TriggerConditional_Factory::Token>* TriggerConditional_Const::get_infix_tokenlist( void ) {
   std::vector< TriggerConditional_Factory::Token >* retval = new std::vector< TriggerConditional_Factory::Token >;

   TriggerConditional_Factory::Token tok;
   tok.token = TriggerConditional_Factory::TRIGGER;
   tok.data  = m_trigger;
   retval->push_back( tok );
   return retval;
}


/*
 * TriggerConditional_TwoArg
 */
TriggerConditional_TwoArg::TriggerConditional_TwoArg(TriggerConditional* l, TriggerConditional* r) {
   m_lconditional = l;
   m_rconditional = r;
}
TriggerConditional_TwoArg::~TriggerConditional_TwoArg( void ) {
   if( m_lconditional )
      delete m_lconditional;
   if( m_rconditional )
      delete m_rconditional;
}
bool TriggerConditional_TwoArg::eval( Game* g ) {
   assert( m_lconditional && m_rconditional );

   bool l_result, r_result;
   l_result = m_lconditional->eval(g );
   r_result = m_rconditional->eval(g );

   return do_eval( l_result, r_result );
}
std::vector< TriggerConditional_Factory::Token >* TriggerConditional_TwoArg::get_infix_tokenlist( void ) {
   std::vector< TriggerConditional_Factory::Token >* retval = new std::vector< TriggerConditional_Factory::Token >;

   TriggerConditional_Factory::Token tok;
   tok.data = 0;

   // (
   tok.token = TriggerConditional_Factory::LPAREN; retval->push_back( tok );

   // Now add our lefts conditionals tokenlist
   std::vector< TriggerConditional_Factory::Token >* l = m_lconditional->get_infix_tokenlist();
   for( uint i = 0; i < l->size(); i++)
      retval->push_back( (*l)[i] );
   delete l;

   // Our token name
   tok.token = get_token();  retval->push_back( tok );

   // Now add our right conditionals tokenlist
   std::vector< TriggerConditional_Factory::Token >* r = m_rconditional->get_infix_tokenlist();
   for( uint i = 0; i < r->size(); i++)
      retval->push_back( (*r)[i] );
   delete r;

   // )
   tok.token = TriggerConditional_Factory::RPAREN; retval->push_back( tok );

   return retval;
}

/*
 * The effective Trigger Conditionals
 */
// AND
TriggerAND::TriggerAND(TriggerConditional* l, TriggerConditional* r)
  : TriggerConditional_TwoArg(l, r) {
}
bool TriggerAND::do_eval( bool t1, bool t2) {
   return (t1 && t2);
}

// OR
TriggerOR::TriggerOR(TriggerConditional* l, TriggerConditional* r)
  : TriggerConditional_TwoArg(l, r) {
}
bool TriggerOR::do_eval( bool t1, bool t2) {
   return (t1 || t2);
}

// XOR
TriggerXOR::TriggerXOR(TriggerConditional* l, TriggerConditional* r)
  : TriggerConditional_TwoArg(l, r) {
}
bool TriggerXOR::do_eval( bool t1, bool t2) {
   return ( (t1 && !t2) || (!t1 && t2) );
}

// NOT
TriggerNOT::TriggerNOT(TriggerConditional* cond) :
   TriggerConditional_OneArg( cond ) {
}
bool TriggerNOT::do_eval( bool t ) {
   return ( !t );
}
