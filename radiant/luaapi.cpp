#include <stdint.h>
#include <math.h>

#include <luajit-2.1/lua.hpp>
#include "../libs/luabridge/LuaBridge.hpp"
#include "../libs/cpp11stringfmt.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// #include <lua.hpp>
/* #include <stdio.h> */
// #include "lua.h"
// #include "lualib.h"
// #include "lauxlib.h"

#include "brush.h"
#include "clippertool.h"
#include "ibrush.h"
#include "iundo.h"
#include "mainframe.h"
#include "points.h"
#include "select.h"
#include "selection.h"

#include "luaapi.h"

// #include "scenelib.h"

// static instance
// static scene::Instance* instance;

/* #include "./luaaa.hpp" */
/* using namespace luaaa; */

using namespace luabridge;


using BrushRef = std::reference_wrapper<Brush>;
using FaceRef = std::reference_wrapper<Face>;

// {{{1 For Each Selected
// class BrushInstanceWalker : public InstanceWalker<float>
// {
// std::function<void (BrushInstance&)> cb;
// public:
// BrushInstanceWalker( std::function<void (BrushInstance&)> cb ) : cb( cb ) {}


// void visit( scene::Instance& instance ) const {
// 	BrushInstance* bi = InstanceTypeCast<BrushInstance>::cast( instance );
// 	if ( bi ) {
// 		// cb(static_cast<BrushInstance&>(*bi));
// 		cb( *bi );
// 	}
// }
// };
class ForEachBrushSelectedInstance : public SelectionSystem::Visitor
{
std::function<void (BrushInstance&)> cb;
public:
ForEachBrushSelectedInstance( std::function<void (BrushInstance&)> cb ) : cb( cb ) {}


void visit( scene::Instance& instance ) const {
	BrushInstance* bi = InstanceTypeCast<BrushInstance>::cast( instance );
	if ( bi ) {
		// cb(static_cast<BrushInstance&>(*bi));
		cb( *bi );
	}
}
};
// using ForEachSelectedBrushCb = std::function<void ( Brush &b )>;
class ForEachSelectedBrush : public SelectionSystem::Visitor
{
std::function<void (Brush&)> cb;
// ForEachSelectedBrushCb cb;
// lua_State *L;
// int *j;
public:
ForEachSelectedBrush( std::function<void (Brush&)> cb ) : cb( cb ) {}
// ForEachSelectedBrush() {}
// ForEachSelectedBrush( ForEachSelectedBrushCb _cb ) : cb( _cb ) {}
// ForEachSelectedBrush( lua_State *L, int *j ) : L( L ),j( j ) {}


void visit( scene::Instance& instance ) const {
	BrushInstance* bptr = InstanceTypeCast<BrushInstance>::cast( instance );
	if ( bptr ) {
		Brush& brush( bptr->getBrush() );
		cb( brush );
	}

	// BrushInstance* bptr = InstanceTypeCast<BrushInstance>::cast( instance );
	// if ( bptr ) {
	// 	Brush& brush( bptr->getBrush() );
	// 	// cb( brush );
	// 	// LuaBrush lb(brush);
	// 	int poo = 90;
	// 	// auto result = luabridge::push( L, &lb );
	// 	// auto result = luabridge::push( L, lb );
	// 	auto result = luabridge::push( L, &brush );
	// 	// auto result = luabridge::push( L, poo );
	// 	( *j )++;
	// 	// exporter.BeginBrush( brush );
	// 	// ForEachFace face_vis( exporter );
	// 	// brush.forEachFace( face_vis );
	// 	// exporter.EndBrush();
	// }
}
};
// }}}1
// {{{1 For Each Selected
class ForEachFace : public BrushVisitor
{
std::function<void (Face&)> cb;
public:
ForEachFace( std::function<void (Face&)> cb ) : cb( cb ) {}

void visit( Face& face ) const {
	cb( face );
}
};
// }}}1


// Concrete templated classes
using Vector3F = BasicVector3<float>;
using Vector3D = BasicVector3<double>;
using Vector3D3 = BasicVector3<double>[3];
using Plane3 = Plane3___<double>;

auto brushInstanceGetFaces = [] (BrushInstance* b) -> std::vector<Face*> {
								 std::vector<Face*> result;

								 auto cb = [&result] (Face& f) {
											   result.push_back( &f );
										   };

								 b->getBrush().forEachFace( ForEachFace( cb ) );

								 return result;
							 };

auto faceGetTexture = [] (Face& f) {
						  return f.getShader().getShader();
					  };

auto brushInstanceGetNumber = [] (BrushInstance &bi) {
	// @TODO
	// return bi->m_counter
	// 							  return f.getShader().getShader();
};

// https://github.com/kunitoki/LuaBridge3
int lad_init_luaapi( lua_State * L ) {

	// auto brushGetFaces = [] (Brush* b, luabridge::LuaRef r) -> std::vector<Face*> {
	auto brushGetFaces = [] (Brush* b) -> std::vector<Face*> {
							 std::vector<Face*> result;

							 auto cb = [&result] (Face& f) {
										   result.push_back( &f );
									   };

							 b->forEachFace( ForEachFace( cb ) );

							 return result;
						 };

	// auto getSelectedNew = [] () -> std::vector<BrushRef> {
	// 						  std::vector<BrushRef> result = {};
	// auto selectedBrushes = [] () -> auto {
	// 						   std::vector<Brush*> result = {};

	// 						   auto selectedCount = GlobalSelectionSystem().countSelected();
	// 						   if ( selectedCount <= 0 ) {
	// 							   return result;
	// 						   }

	// 						   auto cb = [&result] (Brush& b) {
	// 										 result.push_back( &b );
	// 									 };

	// 						   GlobalSelectionSystem().foreachSelected( ForEachSelectedBrush( cb ) );

	// 						   return result;
	// 					   };

	auto selectedBrushInstances = [] () -> auto {
									  std::vector<BrushInstance*> result = {};

									  // GlobalSelectionSystem()
									  auto selectedCount = GlobalSelectionSystem().countSelected();
									  if ( selectedCount <= 0 ) {
										  return result;
									  }

									  auto cb = [&result] (BrushInstance& b) {
													result.push_back( &b );
												};

									  GlobalSelectionSystem().foreachSelected( ForEachBrushSelectedInstance( cb ) );

									  return result;
								  };

	auto radForEachBrushInstances = [] () -> auto {
										std::vector<BrushInstance*> result = {};

										// auto cb = [&result] (BrushInstance& b) {
										// auto cb = [&result] (scene::Instance& i) -> void {
										// 			  BrushInstance* bi = InstanceTypeCast<BrushInstance>::cast( i );
										// 			  result.push_back( bi );
										// 		  };
										auto cb = [&result] (BrushInstance& bi) {
													  result.push_back( &bi );
												  };

										// GlobalSelectionSystem().foreachSelected( ForEachBrushInstance( cb ) );
										// Scene_forEachBrush( GlobalSceneGraph(),  ForEachBrushInstance( cb ) );
										// Scene_forEachBrush( GlobalSceneGraph(),  InstanceWalker< InstanceApply<BrushInstance, std::function<void(scene::Instance*)> >( cb ) );
										// auto boo = InstanceApply<BrushInstance*, std::function<void(BrushInstance*)>>(cb);
										// auto boo = InstanceApply<BrushInstance, std::function<void(BrushInstance)>>(cb);
										auto boo = InstanceApply<BrushInstance, std::function<void(BrushInstance&)> >( cb );
										Scene_forEachBrush( GlobalSceneGraph(),  boo );
										return result;
									};

	// auto forEachBrush = [] (std::function<void(Brush*)> cb) -> auto {
	// 						Select_ShowAllHidden();
	// 					};

	// auto radBrushInstanceClip = [] (BrushInstance& self, std::array<double, 4> vals) {
	// 								return self.setClipPlane( Plane3( vals[0], vals[1], vals[2], vals[3] ) );
	// 							};
	// auto radBrushInstanceClip = [] (BrushInstance& self,  const Vector3D planepts[3] ) {
	// auto radBrushInstanceClip = [] (BrushInstance& self, std::array<Vector3D, 3> vals) {
	// auto radBrushInstanceClip = [] (BrushInstance& self, ClipperPoints& pts) {
	// 								// const Vector3D planepts[3] = {vals[0], vals[1], vals[2]};
	// 								// Plane3 p3 = plane3_for_points( planepts );
	// 								// self.setClipPlane( p3 );
	// 								ClipperMode();
	// 								Clipper_setPlanePoints( pts );
	// 								Clipper_doClip();
	// 								ClipperMode();
	// 							};
	auto radBrushInstanceClipp = [] (BrushInstance& self, std::array<double, 4> vals) {
								 };




	// auto floatVector3Ctor = [] (float x, float y, float z) -> auto { return Vector3(x, y, z); };

	/* *INDENT-OFF* */
	auto radUnhide             = [] ()                  -> auto { Select_ShowAllHidden(); };
	auto radSelectedHide       = [] ()                  -> auto { HideSelected(); };
	auto radSelectedDelete     = [] ()                  -> auto { Select_Delete(); };
	// auto brushInstanceCounter  = [] (BrushInstance& bi) -> auto { bi.m_counter };
	/* *INDENT-ON* */

	// auto vector3dToString = [] (Vector3D &self) { return string_format("Vector3D({%F, %F, %F})", self.x(), self.y(), self.z()); };
	auto vector3dToString = [] (Vector3D &self) {
								std::stringstream ss; ss << "Vector3D({" << self.x() << ", " << self.y() << ", " <<  self.z() << "})"; return ss.str();
							};
	auto vector3fToString = [] (Vector3F &self) {
								std::stringstream ss; ss << "Vector3D({" << self.x() << ", " << self.y() << ", " <<  self.z() << "})"; return ss.str();
							};

	// https://kunitoki.github.io/LuaBridge3/Manual
	/* *INDENT-OFF* */
    luabridge::getGlobalNamespace(L)
        // {
        .beginClass<Vector3F>("Vector3F")
            .addConstructor<void(const float& x_, const float& y_, const float& z_)> ()
            // .addFunction("data", static_cast<float*(Vector3F::*)()>(&Vector3F::data))
            // .addFunction("data", [](Vector3F &self) { return self.data(); })
            // .addFunction ("__tostring", [](Vector3F &self) { return string_format("{%F, %F, %F}", self.x(), self.y(), self.z()); })
            .addConstructor([] (void* ptr, std::array<float, 3> vals) { return new (ptr) Vector3F(vals[0], vals[1], vals[2]); })
            .addFunction("values", [](Vector3F &self) { auto data = self.data(); return std::array<float, 3>{data[0], data[1], data[2]}; })
            .addFunction("setValues", [](Vector3F &self, std::array<float, 3> values) { auto data = self.data(); data[0] = values[0]; data[1] = values[1]; data[2] = values[2]; })
            .addFunction("__tostring", [=](Vector3F &self) { return vector3fToString(self); })
        .endClass()
        .beginClass<Vector3D>("Vector3D")
            // .addConstructor<void(const double& x_, const double& y_, const double& z_)> ()
            // .addConstructor([] (void* ptr, std::array<double, 3> *vals) { return new (ptr) Vector3D(vals->at(0), vals->at(1), vals->at(2)); })
            // .addConstructor([] (std::array<double, 3> *vals) { return Vector3D(vals->at(0), vals->at(1), vals->at(2)); })
            // .addConstructor([] (std::array<double, 3> *vals) { return Vector3D(vals->at(0), vals->at(1), vals->at(2)); })
            .addConstructor([] (void* ptr, std::array<double, 3> vals) { return new (ptr) Vector3D(vals[0], vals[1], vals[2]); })
            .addFunction("add", [](Vector3D &self, Vector3D &other) {
				auto selfData  = self.data();
				auto otherData = other.data();
				selfData[0] += otherData[0];
				selfData[1] += otherData[1];
				selfData[2] += otherData[2];
			})
            .addFunction("getValues", [](Vector3D &self) { auto data = self.data(); return std::array<double, 3>{data[0], data[1], data[2]}; })
            .addFunction("setValues", [](Vector3D &self, std::array<double, 3> values) { auto data = self.data(); data[0] = values[0]; data[1] = values[1]; data[2] = values[2]; })
            .addFunction("__tostring", [=](Vector3D &self) { return vector3dToString(self); })
            .addFunction("toVector3F", [](Vector3D &self) { return Vector3F(self.x(), self.y(), self.z()); })
			// .addIndexMetaMethod ([] (Vector3D& self, const luabridge::LuaRef& key, lua_State* L)
			// {
			// 	if (key.tostring () == "existingProperty")
			// 		return luabridge::LuaRef (L, self.data);

			// 	return luabridge::LuaRef (L, luabridge::LuaNil ()); // or luaL_error("Failed lookup of key !")
			// })
        .endClass()
        .beginClass<Matrix4>("Matrix4")
            .addConstructor([] (void* ptr, std::array<float, 16> vals) { return new (ptr) Matrix4( vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14], vals[15]); })
            .addFunction("index", static_cast<float&(Matrix4::*)(std::size_t)>(&Matrix4::index))
            // .addFunction("index", [](Matrix4 &obj, std::size_t i) { return obj.index(i); })
            // .addFunction("index", [](Matrix4 &obj, std::size_t i) { return obj.index(i); })
        .endClass()
        // classes
        .beginClass<ClipperPoints>("ClipperPoints")
            // .addConstructor([] (std::array<double, 3> vals) { return Vector3D(vals[0], vals[1], vals[2]); })
            // .addConstructor<void(const float& x_, const float& y_, const float& z_)> ()
            // .addConstructor<void(const float& x_, const float& y_, const float& z_)> ()
            // .addConstructor<void( const DoubleVector3& p0, const DoubleVector3& p1, const DoubleVector3& p2, std::size_t count)> ()
            .addConstructor([] (void* ptr, const DoubleVector3& p0, const DoubleVector3& p1, const DoubleVector3& p2, std::size_t count) { return new (ptr) ClipperPoints(p0, p1, p2, count); })
		    .addProperty ("count", &ClipperPoints::_count)
            .addFunction("setPoints", [](ClipperPoints &self, Vector3D x, Vector3D y, Vector3D z, size_t count) {
				self._points[0] = x,
				self._points[1] = y,
				self._points[2] = z,
				self._count = count;
			})
		    // .addProperty ("points", &ClipperPoints::_points)
            .addFunction ("__tostring", [=](ClipperPoints &self) {
				std::string p1 = vector3dToString(self._points[0]);
				std::string p2 = vector3dToString(self._points[1]);
				std::string p3 = vector3dToString(self._points[2]);
				// auto result = string_format("ClipperPoints{{%s, %s, %s}, %d}",
				// 	p1,
				// 	p2,
				// 	p3,
				// 	self._count);
				// return std::string(result.c_str());
				std::stringstream result; result << "ClipperPoints(" << p1 << ", " << p2 << ", " << p3 << ", " << self._count << ")"; return result.str();
			})
        .endClass()
        // .beginClass<Brush>("Brush")
        //     /* .addFunction("getPlanePoints", [](Brush &obj) { return obj.m_move_planepts; }) */
        //     .addFunction("size", &Brush::size)
        //     .addFunction("select", &Brush::size)
        //     .addFunction("clear", &Brush::clear)
        //     .addFunction("delete", &Brush::clear)
        //     // .addFunction("getFaces", brushGetFaces)
        // .endClass()
        // .beginClass<FacePlane>("FacePlane")
        //     // .addFunction("getPlanePoints", &FacePlane::getPlanePoints)
        //     // .addFunction("getPlanePoints", [](Face &self) { return self.m_move_planepts; })
        //     // .addFunction("getPlane", static_cast<FacePlane&(Face::*)()>(&Face::getPlane))
        //     // .addFunction("getTexDef", static_cast<FaceTexdef&(Face::*)()>(&Face::getTexdef))
        //     // .addFunction("isFiltered", &Face::isFiltered)
        //     // .addFunction("transform", &Face::transform)
        //     // .addFunction("getPlane", static_cast<FacePlane& (*)()>(&Face::getPlane))
        // .endClass()
        .beginClass<Face>("Face")
            // .addFunction("getPlanePoints", [](Face &self) { return self.m_move_planepts; })
            .addFunction("getPlanePoints", [](Face &self) { auto pp = self.getPlane().getPlanePoints(); return std::array<Vector3D, 3>{ Vector3D(pp[0]), Vector3D(pp[1]), Vector3D(pp[2]) }; })
            .addFunction("getFacePlane", static_cast<FacePlane&(Face::*)()>(&Face::getPlane))
            .addFunction("getPlane", [](Face &self) { return self.getPlane().plane3(); })
            .addFunction("getNormalVector", [](Face &self) -> Vector3D { return self.getPlane().plane3().normal(); })
            // .addFunction("getTexture", static_cast<FaceTexdef&(Face::*)()>(&Face::getTexdef))
            .addFunction("getTexture", faceGetTexture)
            .addFunction("getShader", static_cast<FaceShader&(Face::*)()>(&Face::getShader))
            .addFunction("isFiltered", &Face::isFiltered)
            // .addFunction("transform", &Face::transform)
            // .addFunction("getPlane", static_cast<FacePlane& (*)()>(&Face::getPlane))
        .endClass()
        .beginClass<BrushInstance>("BrushInstance")
            /* .addFunction("getPlanePoints", [](Brush &obj) { return obj.m_move_planepts; }) */
            // .addFunction("size", &Brush::size)
            // .addFunction("select", &Brush::size)
            // .addFunction("clear", &Brush::clear)
            // .addFunction("delete", &Brush::clear)
            // .addFunction("hide", brushInstanceGetFaces) // @TODO
            // .addFunction("clip", radBrushInstanceClip)
            .addFunction("select",   [] (BrushInstance& self) -> auto { self.setSelected(true);  })
            .addFunction("deselect", [] (BrushInstance& self) -> auto { self.setSelected(false); })
            .addFunction("getFaces", brushInstanceGetFaces)
            // .addFunction("transform", &BrushInstance::transformComponents)
            .addFunction("translate", [](BrushInstance& self, const Vector3F& value) {
				// printf("T1\n");
				auto trans = Instance_getTransformable(self);
				// printf("T2\n");
				trans->setTranslation(value);
				// printf("T3\n");
				self.applyTransform();
				// self.update_selected();
				trans->freezeTransform();
				// printf("T4\n");
			})
            .addFunction ("__tostring", [](BrushInstance &self) { return string_format("Brush[%d]", self.m_counter); })
        .endClass()
        .beginClass<Plane3>("Plane3")
        .endClass()
        .beginClass<Transformable>("Transformable")
            .addFunction("setTranslation", [](Transformable& self, const Vector3F& value) { self.setTranslation(value); })
			/* virtual void setTranslation( const Translation& value ) = 0; */
        .endClass()
        .beginClass<FaceTexdef>("FaceTexdef")
            // .addFunction("getPlane", static_cast<FacePlane&(Face::*)()>(&Face::getPlane))
            // .addFunction("getTexture", static_cast<FaceTexdef&(Face::*)()>(&Face::getTexdef))
            // .addFunction("isFiltered", &Face::isFiltered)
        .endClass()
        // .addFunction("clipper", radClipper)
        // }
		// types
        .beginNamespace("rad")
			.addFunction("newPlanePoints", [](std::array<double, 3> vals) {
				PlanePoints pp;
				return Vector3D(vals[0], vals[1], vals[2]);
			})
			// .addFunction("newVector3D", [](std::array<double, 3> vals) {
			// 	return Vector3D(vals[0], vals[1], vals[2]);
			// })
			.addFunction("drawPlane3", [](Plane3& p) {
				// @TODO
				// return Vector3D(vals[0], vals[1], vals[2]);
			})
			// .addFunction("getClipPoints", []() { auto p = Clipper_getPlanePoints(); return std::array<Vector3D, 3>{p._points[0], p._points[1], p._points[2]}; })
			.addFunction("getClipPoints", []() { return Clipper_getPlanePoints(); })
			.addFunction("clip", [](ClipperPoints& pts, bool flip) {
                printf("%F\n", pts._points[0].x());
                printf("%F\n", pts._points[0].y());
                printf("%F\n", pts._points[0].z());

                ClipperMode();
				Clipper_setPlanePoints(pts);
				// if (flip) {
				// 	Clipper_doFlip();
				// }
				Clipper_doClip();
                ClipperMode();
			})
			// .addFunction("setClipPoints", []() { auto p = Clipper_getPlanePoints(); return std::array<Vector3D, 3>{p._points[0], p._points[1], p._points[2]}; })
			.addFunction("allBrushes", radForEachBrushInstances)
			.addFunction("selectedBrushes", selectedBrushInstances)
			.addFunction("selectedDelete", radSelectedDelete)
			.addFunction("selectedHide", radSelectedHide)
			.addFunction("deselectAll", []() { Selection_Deselect(); })
			.addFunction("unhide", radUnhide)
			.beginNamespace("points")
				.addFunction("empty", []() { Pointfile_Destroy(); Pointfile_Construct(); })
				.addFunction("construct", &Pointfile_Construct)
				.addFunction("insert", &Pointfile_Insert)
				.addFunction("show", &Pointfile_Show)
			.endNamespace()
			.addFunction("getBrushNumber", [](BrushInstance &bi) {
				return brushInstanceGetNumber(bi);
			})
			.addFunction("showMessageBox", [](const char* title, const char* msg) {
				QDialog dialog( nullptr, Qt::Window );
				// QMessageBox::StandardButton overwrite = QMessageBox::StandardButton::Yes;
				int result = (QMessageBox::StandardButton)QMessageBox(
					QMessageBox::Icon::Information,
					QString(title),
					QString(msg),
					QMessageBox::StandardButton::Ok,
					&dialog
				).exec();
				return result;
			})
			.addFunction("showBrushDetails", [](BrushInstance& bi) {
				QInputDialog qDialog ;
				QDialog dialog( nullptr, Qt::Window );
				QVBoxLayout vbox(&dialog);

				{


				}

				auto qlabel1 = new QLabel();
				qlabel1->setText(QString("Number: TBD"));
				auto qlabel2 = new QLabel();
				qlabel2->setText(QString((bi.getBrush().isDetail()) ? "Type: detail" : "Type: structural"));

				auto listw = new QListWidget();
				listw->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
				auto faces = brushInstanceGetFaces(&bi);
				for (auto face : faces) {
					listw->addItem(QString(faceGetTexture(*face)));
				}

				// listw->addItem("jqwe");
				// listw->addItem("43");
				// listw->addItem("oiqwoe");
				// listw->addItem("yxcy");
				listw->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

				// QString resultStr = "";
				std::vector<std::string> result = {};

				{
					// auto btn = new QPushButton();
					// // auto bc = &QPushButton::click;
					// QObject::connect(btn, &QPushButton::click, [&dialog, &listw, &resultStr]() {
					// 		resultStr = listw->currentItem()->text();
					// 		dialog.finished(0);
					// 		});
					auto buttons = new QDialogButtonBox( QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel );
					vbox.addWidget(buttons);
					QObject::connect(buttons, &QDialogButtonBox::accepted, [&dialog, &listw, &result]() {
						auto selectedItems = listw->selectedItems().toStdList();
						for (auto selectedItem : selectedItems) {
							if (selectedItem != nullptr) {
								auto text = selectedItem->text();
								if (text != nullptr) {
									result.push_back(text.toStdString());
								}
							}
						}
						// resultStr = listw->currentItem()->text();
						// resultStr = QString("JKALSD");
						dialog.done(0);
					});
				}

				vbox.addWidget(qlabel1);
				vbox.addWidget(qlabel2);
				vbox.addWidget(listw);
				// vbox.addWidget(btn);

				dialog.exec();
				// return resultStr.toStdString();
				return result;
				// if () {
				// 	return "";
				// } else {
				// 	return "";
				// }
			})
			.addFunction("showComboBox", [](std::vector<std::string> items) {
				// QDialog dialog( nullptr, Qt::Window );
				// // QMessageBox::StandardButton overwrite = QMessageBox::StandardButton::Yes;
				// auto result = QComboBox();
				// result.addItem("foo");
				// result.addItem("bar");
				// // .exec();
				// return result;

				QInputDialog qDialog ;
				QDialog dialog( nullptr, Qt::Window );
				// QListWidget()
				QVBoxLayout vbox(&dialog);
				// QListWidget listw(&dialog);
				auto listw = new QListWidget();
				listw->addItem("jaksd");
				listw->addItem("jqwe");
				listw->addItem("43");
				listw->addItem("oiqwoe");
				listw->addItem("yxcy");
				listw->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
				auto btn = new QPushButton();
				// // auto combo = new QComboBox();
				// vbox.addWidget(listw);
				// vbox.addWidget(btn);

				vbox.addWidget(listw);
				vbox.addWidget(btn);

				// auto combo = new QComboBox();
				// combo->addItem("asjkd");
				// combo->addItem("ioqwue");
				// QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, "Select value", "Select the value", QMessageBox::Ok | QMessageBox::Cancel);
				// QVBoxLayout vlayout;
				// vlayout.addWidget(listw);
				// vlayout.setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);
				// box->setLayout(&vlayout);
				// QGridLayout *layout = (QGridLayout*)box->layout();
				// layout->addWidget(listw);
				// layout->setSizeConstraint(QGridLayout::SetMaximumSize);

// mainLayout.setSizeConstraint(QLayout::SetFixedSize);
				// layout->addWidget(listw, 1,2);
				// int ret = box.exec();

// QVBoxLayout * vbox = new QVBoxLayout();
				// dialog.setLayout(vbox);


				// QStringList qitems;
				// for (auto item : items) {
				// 	qitems << QString::fromStdString(item);
				// }
				// // qitems << QString("Choice 2");
				// // qitems << QString("Choice 3");

				// qDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
				// qDialog.setComboBoxItems(qitems);
				// qDialog.setWindowTitle("Choose action");

				// // QObject::connect(&qDialog, SIGNAL(textValueChanged(const QString &)),
				// // 		   this, SLOT(onCompute(const QString &)));
				// // qDialog.getItem

				// QObject::connect(&dialog, SIGNAL(textValueChanged(const QString &)),
				// 		   this, SLOT(onCompute(const QString &)));
				// QObject::connect(&dialog, SIGNAL(keyPressEvent(QKeyEvent *e)), [](QKeyEvent *e) {});
				// QObject::connect(&dialog, &QDialog::keyPressEvent, [](QKeyEvent *e) {});

				if (dialog.exec()) {
				// if (box->exec()) {
				   // return combo->currentText().toStdString();
				   // return listw->currentItem()->text().toStdString();
					return "";
				} else {
					// return QString("").toStdString().c_str();
					return "";
				}

				// if (dialog.exec()) {
				//    // return dialog.textValue().toStdString();
				//    return listw->currentItem()->text().toStdString();
				// } else {
				// 	return QString("").toStdString();
				// }

				// return qDialog.exec();
			})
            // .beginClass<foo>("foo")
            //     .addFactory(
            //         +[]() -> foo* { return new foo(5000); },
            //         +[](foo* x) { delete x; })
            //     .addFunction("first", &foo::first)
            //     .addFunction("next", &foo::next)
            // .endClass()
            // .beginClass<bar>("bar")
            //     .addConstructor<void(*)(foo *)>()
            //     .addFunction("first", &bar::first)
            //     .addFunction("next", &bar::next)
            // .endClass()
        .endNamespace()
        // .beginNamespace("ose")
			// .addFunction("sleep", [](int i) { Sys_Sleep(i); })
        // .endNamespace()
		;
	/* *INDENT-ON* */

	return 0;
}
