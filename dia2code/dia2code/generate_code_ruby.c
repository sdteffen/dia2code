
/***************************************************************************
        generate_code_ruby.c  -  Function that generates Ruby code
                             -------------------
    begin                : Sun Jun 1 2003
    copyright            : (C) 2003 by Dmitry V. Sabanin
    email                : sdmitry@lrn.ru
    minor modifications  : (C) 2006 okellogg@users.sourceforge.net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *   Comments:
 *   interface are implemented via Mixins
 *   packages not implemented yet
***************************************************************************/

#include "dia2code.h"

#define TABS "  "  /* 2 */

void generate_code_ruby(batch *b) {
    umlclasslist tmplist, parents, incparent;
    umlassoclist associations;
    umlattrlist umla, tmpa, parama;
    umlpackagelist tmppcklist;
    umloplist umlo;
    char *tmpname;
    char outfilename[90];
    FILE * outfile, *dummyfile, *licensefile = NULL;
    umlclasslist used_classes;

    int tmpdirlgth, tmpfilelgth;

    if (b->outdir == NULL) {
        b->outdir = ".";
    }
    tmpdirlgth = strlen(b->outdir);

    tmplist = b->classlist;

    /* open license file */
    if ( b->license != NULL ) {
        licensefile = fopen(b->license, "r");
        if(!licensefile) {
            fprintf(stderr, "Can't open the license file.\n");
            exit(2);
        }
    }

    while ( tmplist != NULL ) {

        if ( ! ( is_present(b->classes, tmplist->key->name) ^ b->mask ) ) {
            char *pretty_outfilename;

            tmpname = tmplist->key->name;

            /* This prevents buffer overflows */
            tmpfilelgth = strlen(tmpname);
            if (tmpfilelgth + tmpdirlgth > sizeof(*outfilename) - 2) {
                fprintf(stderr, "Sorry, name of file too long ...\nTry a smaller dir name\n");
                exit(4);
            }

            pretty_outfilename = strtolower(tmplist->key->name);
            sprintf(outfilename, "%s/%s.class.rb", b->outdir, pretty_outfilename);

            dummyfile = fopen(outfilename, "r");
            if ( b->clobber || ! dummyfile ) {
                int have_parent = 0;

                outfile = fopen(outfilename, "w");
                if ( outfile == NULL ) {
                    fprintf(stderr, "Can't open file %s for writing\n", outfilename);
                    exit(3);
                }
/* header */
                /* add license to the header */
                if (b->license != NULL) {
                    char lc;
                    rewind(licensefile);
                    while((lc = fgetc(licensefile)) != EOF){
                        fprintf(outfile,"%c",lc);
                    }
                }

                fprintf(outfile,"#!/usr/bin/env ruby\n#Source generated by dia2code.\n\n" );
                /* We generate the include clauses */
                used_classes = list_classes(tmplist, b);
                while (used_classes != NULL) {
                    tmppcklist = make_package_list(used_classes->key->package);
                    if ( tmppcklist != NULL ){
                        if ( strcmp(tmppcklist->key->id,tmplist->key->package->id)){
                            /* This class' package and our current class' package are
                               not the same */
                            fprintf(outfile, "include ");
                            fprintf(outfile,"%s",tmppcklist->key->name);
                            tmppcklist=tmppcklist->next;
                            while (tmppcklist != NULL) {
                                fprintf(outfile, "%s", tmppcklist->key->name);
                                tmppcklist=tmppcklist->next;
                            }
                            fprintf(outfile,"%s\n",used_classes->key->name);
                        }
                    } else {
                        /* No info for this class' package, we include it directly
                        fprintf(outfile, "require \"%s.class.rb\"\n",strtolower(used_classes->key->name));
                         */
                    }
                    used_classes = used_classes->next;
                }

                incparent = tmplist->parents;
                if (incparent != NULL) {
                    while (incparent!= NULL) {
                        tmpname = strtolower(incparent->key->stereotype);
                        if (eq("interface", tmpname) ) {
                            if (strcmp("Enumerable",incparent->key->name) && strcmp("Comparable",incparent->key->name)) {
                                fprintf(outfile, "require \"");
                                fprintf(outfile, "%s.module.rb\" \n", strtolower(incparent->key->name));
                            } else {
                                break;
                            }
                        } else {
                            fprintf(outfile, "require \"");
                            fprintf(outfile, "%s.class.rb\" \n", strtolower(incparent->key->name));
                        }
                        free(tmpname);
                        incparent = incparent->next;
                    }
                }
                fprintf(outfile,"\n\n");

                fprintf(outfile,"# %s - short description\n", pretty_outfilename);
                fprintf(outfile,"# Author:: <authors@email.com>\n");
                fprintf(outfile,"# Copyright:: (C) XXX\n");
                fprintf(outfile,"# License:: GPL\n");

                tmppcklist = make_package_list(tmplist->key->package);
                if ( tmppcklist != NULL ){
                    int packcounter = 0;
                    /*
                    fprintf(outfile,"# == MODULES\n#module %s",tmppcklist->key->name);
                    tmppcklist=tmppcklist->next;
                    while ( tmppcklist != NULL ){
                        if( packcounter == 1 ) {
                           fprintf(outfile,"\n" );
                           fprintf(outfile,"# === SUBMODULES\n# submodule %s",tmppcklist->key->name);
                        } else {
                           fprintf(outfile,".%s",tmppcklist->key->name);
                        }
                        tmppcklist=tmppcklist->next;
                        packcounter++;
                    }
                    fprintf(outfile,"\n");
                     */
                }

                tmpname = strtolower(tmplist->key->stereotype);
/*
                if (eq("interface", tmpname)) {
                    fprintf(outfile," * @interface\n" );
                } else {
*/

                    if ( strlen(tmplist->key->comment) > 0 ) {
                        fprintf(outfile,"# %s\n",tmplist->key->comment);
                    } else {
                        fprintf(outfile,"# XXX\n",tmplist->key->comment);
                    }

                    if (tmplist->key->isabstract) {
                        fprintf(outfile,"# * this class is abstract\n" );
                    }
/*
                }
*/
                fprintf(outfile,"\n");
                free(tmpname);

                fprintf(outfile, "class %s", tmplist->key->name);
                parents = tmplist->parents;
                if (parents != NULL) {
                    while ( parents != NULL ) {
                        tmpname = strtolower(parents->key->stereotype);
                        if (strcmp("interface", tmpname)) {
                            /* printf("We're in: %s, type: %s",parents->key->name,tmpname); */
                            if (have_parent) {
                                fprintf(outfile, " # can't inherit %s\n", parents->key->name);
                            } else {
                                fprintf(outfile, " < ");
                                fprintf(outfile, "%s", parents->key->name);
                                have_parent = 1;
                            }
                        }
                        free(tmpname);
                        parents = parents->next;
                    }
                }
                free(parents);
                parents = tmplist->parents;
                fprintf(outfile,"\n");
                if (parents != NULL) {
                    while ( parents != NULL ) {
                        tmpname = strtolower(parents->key->stereotype);
                        if (eq("interface", tmpname)) {
                            fprintf(outfile,"%sinclude %s\n",TABS,parents->key->name);
                        }
                        free(tmpname);
                        parents = parents->next;
                    }
                }
                fprintf(outfile, "\n");

                umla = tmplist->key->attributes;
                while (umla != NULL) {

                    if ( strlen(umla->key.comment) > 0 ) {
                        fprintf(outfile, "%s# %s \n", TABS, umla->key.comment);
                    } else {
                        fprintf(outfile, "%s# XXX \n", TABS);
                    }

                    fprintf(outfile, "%s# * access ", TABS);
                    switch (umla->key.visibility) {
                    case '0':
                        fprintf (outfile, "public");
                        break;
                    case '1':
                        fprintf (outfile, "private");
                        break;
                    case '2':
                        fprintf (outfile, "protected");
                        break;
                    }
                    fprintf(outfile,"\n" );
/*
                    if (umla->key.isstatic) {
                        fprintf(outfile, "static ");
                    }
*/
/*                    fprintf(outfile, "%s %s", umla->key.type, umla->key.name);
*/
                    if ( umla->key.value[0] != 0 ) {
                        fprintf(outfile, "%s@%s = %s\n", TABS, umla->key.name, umla->key.value);
                    }
                    if ( umla->key.isstatic ) {
                        fprintf(outfile, "%s@@%s = nil\n", TABS, umla->key.name );
                    }

                    fprintf(outfile, TABS);
                    if (umla->key.visibility == '0') { /* make it public */
                        fprintf(outfile, "attr_acessor :%s\n", umla->key.name);
                        fprintf(outfile, "%spublic :%s\n\n",TABS, umla->key.name);
                    } else if (umla->key.visibility == '1') { /* make it private */
                        fprintf(outfile, "attr :%s\n", umla->key.name);
                        fprintf(outfile, "%sprivate :%s\n\n", TABS, umla->key.name);
                    } else {
                        fprintf(outfile, "attr :%s\n", umla->key.name);
                        fprintf(outfile, "%sprotected :%s\n\n",TABS, umla->key.name);
                    }
                    umla = umla->next;
                }
                associations = tmplist->associations;
                /* I've turned off associations coz they are: not working and
                 * i can't think of how it could be used in source
                 */
                while ( associations != NULL && 0) {
                    fprintf(outfile, "# association %s to %s\n",
                            associations->key->name, associations->name);
                    fprintf(outfile, "%s# access private\n", TABS );
                            fprintf(outfile, "%s# attribute @%s;\n\n", TABS, associations->name);
                    associations = associations->next;
                }
                umlo = tmplist->key->operations;

                while ( umlo != NULL) {
                    int bracket_opened = 0;

                    if ( strlen(umlo->key.attr.comment) > 0 ) {
                        fprintf(outfile,"%s# %s\n", TABS, umlo->key.attr.comment );
                    } else {
                        fprintf(outfile,"%s# XXX\n", TABS );
                    }

                    if ( umlo->key.attr.isabstract ) {
                        fprintf(outfile,"%s# * abstract\n", TABS );
                        umlo->key.attr.value[0] = '0';
                    }

                    fprintf(outfile,"%s# * access ", TABS );
                    switch (umlo->key.attr.visibility) {
                    case '0':
                        fprintf (outfile, "public ");
                        break;
                    case '1':
                        fprintf (outfile, "private ");
                        break;
                    case '2':
                        fprintf (outfile, "protected ");
                        break;
                    }
                    fprintf(outfile,"\n" );

/*                    if ( umlo->key.attr.isstatic ) {
                        fprintf(outfile, "static ");
                    }
*/
                    if (strlen(umlo->key.attr.type) > 0) {
                        fprintf(outfile,"%s# * returns %s\n", TABS,umlo->key.attr.type);
                    }
                    parama = umlo->key.parameters;
                    /* document parameters */
                    while (parama != NULL) {
                        char *comment = "";
                        if (strlen(parama->key.comment) > 0)
                            comment = parama->key.comment;
                        fprintf(outfile, "%s# * param %s %s %s\n", TABS, parama->key.type, parama->key.name, comment);
                        parama= parama->next;
                    }

                    fprintf(outfile, TABS);
                    fprintf(outfile, "def %s", umlo->key.attr.name);
                    tmpa = umlo->key.parameters;
                    if (tmpa != NULL) {
                        fprintf(outfile, "( ");
                        bracket_opened = 1;
                    }
                    while (tmpa != NULL) {
                        fprintf(outfile, "%s", tmpa->key.name);
                        if ( tmpa->key.value[0] != 0 ){
                            fprintf(outfile," = %s",tmpa->key.value);
                        }
                        tmpa = tmpa->next;
                        if (tmpa != NULL)
                            fprintf(outfile, ", ");
                    }
                    if (bracket_opened) {
                        fprintf(outfile, " )");
                    }
/*                  if ( umlo->key.attr.isabstract ) {
                        fprintf(outfile, ";\n");
                    } else {
*/
                        fprintf(outfile, "\n");
                        if ( umlo->key.implementation != NULL ) {
                            fprintf(outfile, "%s%s\n",TABS, umlo->key.implementation);
                        } else {
                            fprintf(outfile, "%s%sraise NotImplementedError, 'This is auto-gen. method, please implement.'\n",
                                    TABS, TABS);
                        }
                        fprintf(outfile, "%send\n", TABS);
/*                  }
*/
                    switch (umlo->key.attr.visibility) {
                        case '0':
                            fprintf(outfile, "%spublic :%s\n\n", TABS, umlo->key.attr.name);
                            break;
                        case '1':
                            fprintf(outfile, "%sprivate :%s\n\n", TABS, umlo->key.attr.name);
                            break;
                        case '2':
                            fprintf(outfile, "%sprotected :%s\n\n", TABS, umlo->key.attr.name);
                            break;
                    }
                    umlo = umlo->next;
                }
                fprintf(outfile, "end\n\n");

                fprintf(outfile, "\n\n");
                fclose(outfile);
            }
        }
        tmplist = tmplist->next;
    }
}
