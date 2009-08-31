/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Bugzilla Testopia System.
 *
 * The Initial Developer of the Original Code is Greg Hendricks.
 * Portions created by Greg Hendricks are Copyright (C) 2006
 * Novell. All Rights Reserved.
 *
 * Contributor(s): Greg Hendricks <ghendricks@novell.com>
 *                 Ryan Hamilton <rhamilton@novell.com>
 *                 Daniel Parker <dparker1@novell.com>
 */

Testopia.Build.Store = function(params, auto){
    params.action = 'list';
    Testopia.Build.Store.superclass.constructor.call(this, {
        url: 'tr_builds.cgi',
        root: 'builds',
        listeners: { 'exception': Testopia.Util.loadError },
        baseParams: params,
        id: 'build_id',
        autoLoad: auto,
        fields: [{
            name: "id",
            mapping: "build_id"
        }, {
            name: "name",
            mapping: "name"
        }, {
            name: "milestone",
            mapping: "milestone"
        }, {
            name: "description",
            mapping: "description"
        }, {
            name: "product_id",
            mapping: "product_id"
        }, {
            name: "isactive",
            mapping: "isactive"
        }]
    });
};

Ext.extend(Testopia.Build.Store, Ext.data.JsonStore);

/*
 * Testopia.Build.Combo
 */
Testopia.Build.Combo = function(cfg){
    Testopia.Build.Combo.superclass.constructor.call(this, {
        id: cfg.id || 'build_combo',
        store: cfg.transform ? false : new Testopia.Build.Store(cfg.params, cfg.mode == 'local' ? true : false),
        loadingText: 'Looking up builds...',
        displayField: 'name',
        valueField: 'id',
        typeAhead: true,
        triggerAction: 'all',
        minListWidth: 300,
        forceSelection: true,
        transform: cfg.transform,
        emptyText: 'Builds...'
    });
    Ext.apply(this, cfg);
    this.store.on('load', function(){
        if (cfg.value) {
            this.setValue(cfg.value);
        }
    }, this);
};
Ext.extend(Testopia.Build.Combo, Ext.form.ComboBox);

Testopia.Build.Grid = function(product_id){
    this.product_id = product_id;
    this.store = new Testopia.Build.Store({}, false);
    var mbox = new Testopia.Product.MilestoneCombo({
        hiddenField: 'milestone',
        mode: 'remote',
        params: {
            product_id: product_id
        }
    });
    this.columns = [{
        header: "Name",
        width: 80,
        sortable: true,
        dataIndex: 'name',
        editor: new Ext.grid.GridEditor(new Ext.form.TextField({
            value: 'name',
            allowBlank: false
        }), {
            completeOnEnter: true,
            listeners: {
                'beforecomplete': function(e, v){
                    if (!e.getValue()) {
                        return false;
                    }
                }
            }
        })
    }, {
        header: "Milestone",
        width: 120,
        sortable: true,
        dataIndex: 'milestone',
        editor: new Ext.grid.GridEditor(mbox, {
            listeners: {
                'startedit': function(){
                    var pid = Ext.getCmp('products_pane').getSelectionModel().getSelectedNode().id;
                    if (mbox.store.baseParams.product_id != pid) {
                        mbox.store.baseParams.product_id = pid;
                        mbox.store.load();
                    }
                }
            }
        })
    }, {
        header: "Description",
        width: 120,
        editor: new Ext.grid.GridEditor(new Ext.form.TextField()),
        sortable: true,
        dataIndex: 'description'
    }, new Ext.grid.CheckColumn({
        header: 'Active',
        dataIndex: 'isactive',
        editor: new Ext.grid.GridEditor(new Ext.form.Checkbox({
            value: 'isactive'
        })),
        width: 25
    })];
    
    this.form = new Ext.form.BasicForm('testopia_helper_frm');
    
    Testopia.Build.Grid.superclass.constructor.call(this, {
        title: 'Builds',
        id: 'build_grid',
        loadMask: {
            msg: 'Loading Builds...'
        },
        autoExpandColumn: "build_name",
        autoScroll: true,
        sm: new Ext.grid.RowSelectionModel({
            singleSelect: true
        }),
        viewConfig: {
            forceFit: true
        },
        tbar: [new Ext.Toolbar.Fill(), {
            xtype: 'button',
            id: 'edit_build_btn',
            icon: 'extensions/testopia/img/edit.png',
            iconCls: 'img_button_16x',
            tooltip: 'Edit Selected Build',
            handler: function(){
                Testopia.Util.editFirstSelection(Ext.getCmp('build_grid'));
            }
        }, {
            xtype: 'button',
            id: 'add_build_btn',
            icon: 'extensions/testopia/img/add.png',
            iconCls: 'img_button_16x',
            tooltip: 'Add a new Build',
            handler: this.newRecord
        }]
    });
    this.on('rowcontextmenu', this.onContextClick, this);
    this.on('activate', this.onActivate, this);
    this.on('afteredit', this.onGridEdit, this);
};
Ext.extend(Testopia.Build.Grid, Ext.grid.EditorGridPanel, {
    newRecord: function(){
        NewBuild = Ext.data.Record.create([{
            name: 'name',
            type: 'string'
        }, {
            name: 'milestone'
        }, {
            name: 'description',
            type: 'string'
        }, {
            name: 'isactive',
            type: 'bool'
        }]);
        var b = new NewBuild({
            name: '',
            milestone: Ext.getCmp('products_pane').getSelectionModel().getSelectedNode().attributes.attributes.defaultmilestone,
            description: '',
            isactive: true
        });
        var g = Ext.getCmp('build_grid');
        g.store.insert(0, b);
        g.startEditing(0, 0);
    },
    onContextClick: function(grid, index, e){
        grid.getSelectionModel().selectRow(index);
        if (!this.menu) { // create context menu on first right click
            this.menu = new Ext.menu.Menu({
                id: 'build-ctx-menu',
                items: [{
                    text: "Reports",
                    menu: {
                        items: [{
                            text: 'New Completion Report',
                            handler: function(){
                                Ext.getCmp('object_panel').setActiveTab('dashboardpanel');
                                
                                var newPortlet = new Ext.ux.Portlet({
                                    title: 'Build Completion Report',
                                    closable: true,
                                    autoScroll: true,
                                    tools: PortalTools
                                });
                                newPortlet.url = 'tr_builds.cgi?action=report&product_id=' + grid.product_id + '&build_ids=' + Testopia.Util.getSelectedObjects(grid, 'id');
                                Testopia.Search.dashboard_urls.push(newPortlet.url);
                                Ext.getCmp('dashboard_leftcol').add(newPortlet);
                                Ext.getCmp('dashboard_leftcol').doLayout();
                                newPortlet.load({
                                    url: newPortlet.url
                                });
                            }
                        }]
                    }
                }, {
                    text: 'Add a Build',
                    icon: 'extensions/testopia/img/add.png',
                    iconCls: 'img_button_16x',
                    handler: this.newRecord
                }, {
                    text: 'Edit This Build',
                    icon: 'extensions/testopia/img/edit.png',
                    iconCls: 'img_button_16x',
                    handler: function(){
                        Testopia.Util.editFirstSelection(grid);
                    }
                }, {
                    text: 'Refresh',
                    icon: 'extensions/testopia/img/refresh.png',
                    iconCls: 'img_button_16x',
                    handler: function(){
                        grid.store.reload();
                    }
                }]
            });
        }
        e.stopEvent();
        this.menu.showAt(e.getXY());
    },
    onGridEdit: function(e){
        var bid = e.record.get('id');
        var myparams = {
            product_id: this.product_id,
            build_id: bid
        };
        var ds = this.store;
        
        if (bid) {
            myparams.action = "edit";
            switch (e.field) {
                case 'name':
                    myparams.name = e.value;
                    break;
                case 'description':
                    myparams.description = e.value;
                    break;
                case 'isactive':
                    myparams.isactive = e.value;
                    break;
                case 'milestone':
                    myparams.milestone = e.value;
                    break;
            }
        }
        else {
            myparams.action = "add";
            myparams.name = e.value;
            myparams.milestone = Ext.getCmp('products_pane').getSelectionModel().getSelectedNode().attributes.attributes.defaultmilestone;
            myparams.isactive = 1;
        }
        this.form.submit({
            url: "tr_builds.cgi",
            params: myparams,
            success: function(f, a){
                if (a.result.build_id) {
                    e.record.set('id', a.result.build_id);
                }
                ds.commitChanges();
            },
            failure: function(f, a){
                Testopia.Util.error(f, a);
                ds.rejectChanges();
            }
        });
    },
    onActivate: function(event){
        if (!this.product_id) {
            Ext.Msg.alert('Error', 'Please select a product.');
            Ext.getCmp('edit_build_btn').disable();
            Ext.getCmp('add_build_btn').disable();
            return;
        }
        else {
            if (!this.store.getCount()) {
                this.store.load({
                    params: {
                        product_id: this.product_id
                    }
                });
            }
        }
    }
});
