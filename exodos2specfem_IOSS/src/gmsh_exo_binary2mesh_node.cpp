#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

#include "io_info.h"



namespace {
  std::string codename;
  std::string version = "1.0";

  std::string name(Ioss::GroupingEntity *entity)
  {
    return entity->type_string() + " '" + entity->name() + "'";
  }
  
  int64_t id(Ioss::GroupingEntity *entity)
  {
    int64_t id = -1;
    if (entity->property_exists("id")) {
      id = entity->get_property("id").get_int();
    }
    return id;
  }


  void info_aliases(Ioss::Region &region, Ioss::GroupingEntity *ige, bool nl_pre, bool nl_post)
  {
    std::vector<std::string> aliases;
    if (region.get_aliases(ige->name(), aliases) > 0) {
      if (nl_pre) {
        OUTPUT << "\n";
      }
      OUTPUT << "\tAliases: ";
      for (size_t i = 0; i < aliases.size(); i++) {
        if (aliases[i] != ige->name()) {
          if (i > 0) {
            OUTPUT << ", ";
          }
          OUTPUT << aliases[i];
        }
      }
      if (nl_post) {
        OUTPUT << "\n";
      }
    }
  }

  void info_fields(Ioss::GroupingEntity *ige, Ioss::Field::RoleType role, const std::string &header)
  {
    Ioss::NameList fields;
    ige->field_describe(role, &fields);

    if (fields.empty()) {
      return;
    }

    if (!header.empty()) {
      OUTPUT << header;
    }
    // Iterate through results fields and transfer to output
    // database...
    for (const auto &field_name : fields) {
      const Ioss::VariableType *var_type   = ige->get_field(field_name).raw_storage();
      int                       comp_count = var_type->component_count();
      OUTPUT << std::setw(16) << field_name << ":" << comp_count << " ";
    }
    if (!header.empty()) {
      OUTPUT << "\n";
    }
  }


  std::vector<double> get_attributes(int num_atr, Ioss::GroupingEntity *ige){
      std::vector<double> attr;
      size_t isize = ige->get_field("attribute").get_size();

      if (isize != 0){
          //attr.resize(isize);
          attr.resize(num_atr);
          ige->get_field_data("attribute", &attr[0], isize);
  
          /*
          std::cout << "? attribute test" << '\n';
          for (int i = 0; i != attr.size(); ++i){
              std::cout << attr[i] << '\n';
                  else if (i_face ==){}
          */
      }
      return attr;
  }


} // namespace



std::tuple<int, int> info_nodeblock(Ioss::Region &region, const Info::Interface &interface)
{
  Ioss::NodeBlockContainer nbs = region.get_node_blocks();
  int64_t      total_num_nodes = 0;

  int64_t degree = 0;
  for (auto nb : nbs) {
    int64_t num_nodes = nb->get_property("entity_count").get_int();
    total_num_nodes += num_nodes;
    degree = nb->get_property("component_degree").get_int();
  }
  OUTPUT << " Number of spatial dimensions =" << std::setw(12) << degree << "\n";
  OUTPUT << " Number of nodeblocks         =" << std::setw(12) << nbs.size() << "\t";
  OUTPUT << " Number of nodes              =" << std::setw(12) << total_num_nodes << "\n";

  return std::forward_as_tuple(degree, total_num_nodes);
}

void info_elementblock(Ioss::Region &region, const Info::Interface &interface, bool summary)
{
  Ioss::ElementBlockContainer ebs            = region.get_element_blocks();
  int64_t                     total_elements = 0;
  
  for (auto eb : ebs) {
    int64_t num_elem = eb->get_property("entity_count").get_int();
    total_elements += num_elem;

    if (!summary) {
      std::string type       = eb->get_property("topology_type").get_string();
      int64_t     num_attrib = eb->get_property("attribute_count").get_int();
      OUTPUT << '\n'
             << name(eb)   << " id: " << std::setw(6)  << id(eb)   << ", topology: " << std::setw(10)
             << type       << ", "    << std::setw(12) << num_elem << " elements, "  << std::setw(3)
             << num_attrib << " attributes.";

      info_aliases(region, eb, true, false);
      info_fields(eb, Ioss::Field::ATTRIBUTE, "\n\tattributes: ");

      if (interface.adjacencies()) {
        std::vector<std::string> blocks;
        eb->get_block_adjacencies(blocks);
        OUTPUT << "\n\tAdjacent to  " << blocks.size() << " element block(s):\t";
        for (const auto &block : blocks) {
          OUTPUT << block << "  ";
        }
      }
      info_fields(eb, Ioss::Field::TRANSIENT, "\n\tTransient:  ");
      OUTPUT << "\n";

     
      Ioss::AxisAlignedBoundingBox bbox = eb->get_bounding_box();
      OUTPUT << "\tBounding Box: Minimum X,Y,Z = " << std::setprecision(4) << std::scientific
             << std::setw(12) << bbox.xmin << "\t" << std::setw(12) << bbox.ymin << "\t"
             << std::setw(12) << bbox.zmin << "\n"
             << "\t              Maximum X,Y,Z = " << std::setprecision(4) << std::scientific
             << std::setw(12) << bbox.xmax << "\t" << std::setw(12) << bbox.ymax << "\t"
             << std::setw(12) << bbox.zmax << "\n";
    
    }
  }
  if (summary) {
    OUTPUT << " Number of element blocks     =" << std::setw(12) << ebs.size() << "\t";
    OUTPUT << " Number of elements           =" << std::setw(12) << total_elements << "\n";
  }
}


bool check_if_cpml (double &nod_1, double &nod_2, double &nod_3,
                    double &nod_4, double &nod_5, double &nod_6,
                    double &nod_7, double &nod_8, double &b_min, double &b_max) {
    if ( nod_1 <= b_min || nod_2 <= b_min || nod_3 <= b_min || nod_4 <= b_min || nod_5 <= b_min || nod_6 <= b_min || nod_7 <= b_min || nod_8 <= b_min ||
         nod_1 >= b_max || nod_2 >= b_max || nod_3 >= b_max || nod_4 >= b_max || nod_5 >= b_max || nod_6 >= b_max || nod_7 >= b_max || nod_8 >= b_max)
        return true;
    else
        return false;  
}

bool double_equals(double a, double b, double epsilon = 0.0000000001)
{
    return std::abs(a - b) < epsilon;
}

int check_bb_sub (double &nx1, double &ny1, double &nz1,
                  double &nx2, double &ny2, double &nz2,
                  double &nx3, double &ny3, double &nz3,
                  double &nx4, double &ny4, double &nz4,
                  double &x_bmin, double &x_bmax, double &y_bmin,
                  double &y_bmax, double &z_bmin, double &z_bmax ){
    int b_flag = -1;
    double center_x = (nx1+nx2+nx3+nx4)/4.0;
    double center_y = (ny1+ny2+ny3+ny4)/4.0; 
    double center_z = (nz1+nz2+nz3+nz4)/4.0;   

    if (double_equals(center_x, x_bmin))//(center_x == x_bmin)
        b_flag = 0;
    else if (double_equals(center_x, x_bmax))  //(center_x == x_bmax)
        b_flag = 1;
    else if (double_equals(center_y, y_bmin))//(center_y == y_bmin)
        b_flag = 2;
    else if (double_equals(center_y, y_bmax))//(center_y == y_bmax)
        b_flag = 3;
    else if (double_equals(center_z, z_bmin))//(center_z == z_bmin)
        b_flag = 4;
    else if (double_equals(center_z, z_bmax))//(center_z == z_bmax)
        b_flag = 5;

    return b_flag;
}

std::vector<int> check_bb (
                            double &node_1x, double &node_2x, double &node_3x,
                            double &node_4x, double &node_5x, double &node_6x,
                            double &node_7x, double &node_8x,
                            
                            double &node_1y, double &node_2y, double &node_3y,
                            double &node_4y, double &node_5y, double &node_6y,
                            double &node_7y, double &node_8y,
                            
                            double &node_1z, double &node_2z, double &node_3z,
                            double &node_4z, double &node_5z, double &node_6z,
                            double &node_7z, double &node_8z,

                            double &x_bmin, double &x_bmax, double &y_bmin, 
                            double &y_bmax, double &z_bmin, double &z_bmax ) {

    
    std::vector<int> flag_n_ids; // store a flag(0: xmin, ~ 5:zmax) and surf id.
    flag_n_ids.clear(); flag_n_ids.shrink_to_fit();
    // node combination of each surface in exodusII format
    // surf 0: 1,2,6,5
    // surf 1: 2,3,7,6
    // surf 2: 3,4,8,7
    // surf 3: 4,1,5,8
    // surf 4: 4,3,2,1
    // surf 5: 5,6,7,8

    // check surf 0
    flag_n_ids.push_back(check_bb_sub(node_1x, node_1y, node_1z,
                                      node_2x, node_2y, node_2z,
                                      node_6x, node_6y, node_6z,
                                      node_5x, node_5y, node_5z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(0);

    // check surf 1
    flag_n_ids.push_back(check_bb_sub(node_2x, node_2y, node_2z,
                                      node_3x, node_3y, node_3z,
                                      node_7x, node_7y, node_7z,
                                      node_6x, node_6y, node_6z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(1);

    // check surf 2
    flag_n_ids.push_back(check_bb_sub(node_3x, node_3y, node_3z,
                                      node_4x, node_4y, node_4z,
                                      node_8x, node_8y, node_8z,
                                      node_7x, node_7y, node_7z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(2);

    // check surf 3
    flag_n_ids.push_back(check_bb_sub(node_4x, node_4y, node_4z,
                                      node_1x, node_1y, node_1z,
                                      node_5x, node_5y, node_5z,
                                      node_8x, node_8y, node_8z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(3);

    // check surf 4
    flag_n_ids.push_back(check_bb_sub(node_4x, node_4y, node_4z,
                                      node_3x, node_3y, node_3z,
                                      node_2x, node_2y, node_2z,
                                      node_1x, node_1y, node_1z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(4);

    // check surf 5
    flag_n_ids.push_back(check_bb_sub(node_5x, node_5y, node_5z,
                                      node_6x, node_6y, node_6z,
                                      node_7x, node_7y, node_7z,
                                      node_8x, node_8y, node_8z,
                                      x_bmin, x_bmax, y_bmin, y_bmax, z_bmin, z_bmax));
    if (flag_n_ids.back() == -1) flag_n_ids.pop_back();
    else                         flag_n_ids.push_back(5);


    flag_n_ids.shrink_to_fit();
    return flag_n_ids;
}

int main(int argc, char *argv[]) {

    std::cout << "####################################" << '\n';
    std::cout << "# start reading exodus binary file #" << '\n';
    std::cout << "####################################" << '\n';

    /*
    arg1: binary exodus2 file\n 
    arg2: path to MESH folder\n 
    arg3: width of cpml layer
    */

    /* basic properties of this model  */
    int dim = 0;
    int64_t total_num_nodes = 0;
    int total_num_elems = 0;
    int num_volume_block;// = atoi(argv[3]); // number of volume blocks

    double x_bound_min =  99999999;
    double x_bound_max = -99999999;
    double y_bound_min =  99999999;
    double y_bound_max = -99999999;
    double z_bound_min =  99999999;
    double z_bound_max = -99999999;


    Info::Interface interface;
    interface.parse_options(argc, argv);
  
    std::string in_type = "exodusII";
  
    codename   = argv[0];
    size_t ind = codename.find_last_of('/', codename.size());
    if (ind != std::string::npos) {
      codename = codename.substr(ind + 1, codename.size());
    }
  
    Ioss::Init::Initializer io;

    OUTPUT << "Input:    '" << interface.filename() << "', Type: " << interface.type() << '\n';
    OUTPUT << '\n';

    std::string inpfile    = interface.filename();
    std::string input_type = interface.type();

    Ioss::DatabaseIO *dbi  = Ioss::IOFactory::create(input_type, inpfile, Ioss::READ_RESTART, (MPI_Comm)MPI_COMM_WORLD);
    Ioss::io_info_set_db_properties(interface, dbi);

    Ioss::Region region(dbi, "region_1");

    /* get basic properties of this mesh data */
    std::tie(dim, total_num_nodes) = info_nodeblock(region, interface); // the way to return multi vars. heavy thus not for heavy treats


    /* get nodal coordinates  */
    std::vector<double> coordinates; // 3d cartesian in 1d array
    Ioss::NodeBlock *nb = region.get_node_blocks()[0];
    nb->get_field_data("mesh_model_coordinates", coordinates);
    
    
    /* get element block info  */
    info_elementblock(region, interface, false);

    /* get element connectivity */
    std::vector<std::vector<int> > conns; 
    std::vector<int>               num_node_per_elems;
    Ioss::ElementBlockContainer                 ebs = region.get_element_blocks();
    Ioss::ElementBlockContainer::const_iterator ib  = ebs.begin();

    std::vector<int>                  num_elems;
    std::vector<int>                  num_attrs;
    std::vector<std::vector<double> > attrs;

 
    int count_eb = 0;
    int num_vol = 0;

    // first loop to get the number of volume blocks
    while (ib != ebs.end()) {
        Ioss::ElementBlock *eb = *ib;
        ++ib;
 
        std::string type       = eb->get_property("topology_type").get_string();
        if (type.compare(0,3,"hex") == 0)
            num_vol++; 
    }
    num_volume_block = num_vol;
    std::cout << '\n' << "the number of volume blocks: " << num_volume_block << '\n';

    ib = ebs.begin();

    while (ib != ebs.end()) {
        Ioss::ElementBlock *eb = *ib;
        ++ib;
        std::string name = (*eb).name();
      
        int num_elem          = eb->get_property("entity_count").get_int();
        int num_node_per_elem = eb->topology()->number_nodes();
        num_elems.push_back(num_elem);
        num_node_per_elems.push_back(num_node_per_elem);

        std::vector<int> conn;
        //conn.clear(); conn.shrink_to_fit();

        // Get the connectivity array...
        conn.resize(num_elem * num_node_per_elem);

        std::vector<int> eids(num_elem);
        eb->get_field_data("ids", eids);

        eb->get_field_data("connectivity", conn);

        // Connectivity is in global id space; change to local...
        for (int i = 0; i < num_elem * num_node_per_elem; i++) {
            int local = region.node_global_to_local(conn[i]);
            conn[i]   = local - 1;
        }

        // register in conns
        conns.push_back(conn);
        
        
        /* get attributes */
        int64_t     num_attrib = eb->get_property("attribute_count").get_int();
        num_attrs.push_back(num_attrib);
        if (num_attrib != 0)
            attrs.push_back(get_attributes(num_attrib*num_elem, eb));

        if(count_eb < num_volume_block)
            total_num_elems += num_elem;
        
        /*
        //test adjacencies
        if (count_eb >= num_volume_block){
            std::vector<std::string> blocks;
            eb->get_block_adjacencies(blocks);
            OUTPUT << "\n\tAdjacent to  " << blocks.size() << " element block(s):\t";
            for (const auto &block : blocks) {
                OUTPUT << block << "  ";
            }

        }
        */

        //get bounding box coodrinalte
        Ioss::AxisAlignedBoundingBox bbox = eb->get_bounding_box();
        if (bbox.xmin < x_bound_min) x_bound_min = bbox.xmin;
        if (bbox.xmax > x_bound_max) x_bound_max = bbox.xmax;
        if (bbox.ymin < y_bound_min) y_bound_min = bbox.ymin;
        if (bbox.ymax > y_bound_max) y_bound_max = bbox.ymax;
        if (bbox.zmin < z_bound_min) z_bound_min = bbox.zmin;
        if (bbox.zmax > z_bound_max) z_bound_max = bbox.zmax;

        count_eb++;
    }

    
    OUTPUT << "\tTotal Bounding Box: Minimum X,Y,Z = " << std::setprecision(4) << std::scientific
         << std::setw(12) << x_bound_min << "\t" << std::setw(12) << y_bound_min << "\t"
         << std::setw(12) << z_bound_min << "\n"
         << "\t              Maximum X,Y,Z = " << std::setprecision(4) << std::scientific
         << std::setw(12) << x_bound_max << "\t" << std::setw(12) << y_bound_max << "\t"
         << std::setw(12) << z_bound_max << "\n";

    // search the elements, faces and nodes on the bounding box
    // we don't use the surface blocks defined by cubit.
    // at the same time we will check if the elements are in the pml layer
    
    std::vector<int> bb_xmi;
    std::vector<int> bb_ymi;
    std::vector<int> bb_zmi;
    std::vector<int> bb_xma;
    std::vector<int> bb_yma;
    std::vector<int> bb_zma;
    

    // get the thickness of PML layer from command line argument
    double width_cpml = atof(argv[3]);
    
    // define the limits of not-cpml-layer zone
    double x_min = x_bound_min + width_cpml;
    double x_max = x_bound_max - width_cpml;
    double y_min = y_bound_min + width_cpml;
    double y_max = y_bound_max - width_cpml;
    double z_min = z_bound_min + width_cpml;
    double z_max = z_bound_max - width_cpml;

    std::cout << "bounding box excluding cpml layer:"  << '\n';
    std::cout << "x min max" << x_min << ", " << x_max << '\n';
    std::cout << "y min max" << y_min << ", " << y_max << '\n';
    std::cout << "z min max" << z_min << ", " << z_max << '\n';

    std::cout << "cpml width: " << width_cpml << '\n';

    int count_cpml_element = 0;
    std::vector<int> cpml_x;
    std::vector<int> cpml_y;
    std::vector<int> cpml_z;
    std::vector<int> cpml_xy;
    std::vector<int> cpml_xz;
    std::vector<int> cpml_yz;
    std::vector<int> cpml_xyz;

    bool if_x; //flags if the element is included in the cpml boundary zone of each direction
    bool if_y;
    bool if_z;

    int offcet = 0;
    //num_elems.push_back(num_elem);
    //num_node_per_elems.push_back(num_node_per_elem);

    for (int i_vol = 0; i_vol < num_volume_block; ++i_vol){
        int nnpe = num_node_per_elems[i_vol];
        std::cout << "nnpe: " << nnpe << std::endl;

        if (i_vol != 0)
            offcet += num_elems[i_vol-1]; //offcet+j_ele = total element id (starting 0)

        for(int j_ele = 0; j_ele < num_elems[i_vol]; ++j_ele){
      
            // check if this element is in the pml layer
            if_x = false; if_y = false; if_z = false;            

            if_x = check_if_cpml(coordinates[conns[i_vol][j_ele*nnpe]*  dim], coordinates[conns[i_vol][j_ele*nnpe+1]*dim], 
                                 coordinates[conns[i_vol][j_ele*nnpe+2]*dim], coordinates[conns[i_vol][j_ele*nnpe+3]*dim], 
                                 coordinates[conns[i_vol][j_ele*nnpe+4]*dim], coordinates[conns[i_vol][j_ele*nnpe+5]*dim], 
                                 coordinates[conns[i_vol][j_ele*nnpe+6]*dim], coordinates[conns[i_vol][j_ele*nnpe+7]*dim], 
                                 x_min, x_max);

            if_y = check_if_cpml(coordinates[conns[i_vol][j_ele*nnpe]*  dim+1], coordinates[conns[i_vol][j_ele*nnpe+1]*dim+1], 
                                 coordinates[conns[i_vol][j_ele*nnpe+2]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+3]*dim+1], 
                                 coordinates[conns[i_vol][j_ele*nnpe+4]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+5]*dim+1], 
                                 coordinates[conns[i_vol][j_ele*nnpe+6]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+7]*dim+1], 
                                 y_min, y_max);

            if_z = check_if_cpml(coordinates[conns[i_vol][j_ele*nnpe]*  dim+2], coordinates[conns[i_vol][j_ele*nnpe+1]*dim+2], 
                                 coordinates[conns[i_vol][j_ele*nnpe+2]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+3]*dim+2], 
                                 coordinates[conns[i_vol][j_ele*nnpe+4]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+5]*dim+2], 
                                 coordinates[conns[i_vol][j_ele*nnpe+6]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+7]*dim+2], 
                                 z_min, z_max);             

            //x
            if (if_x == true  && if_y == false && if_z == false) {
                cpml_x.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //y
            if (if_x == false && if_y == true  && if_z == false) {
                cpml_y.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //z
            if (if_x == false && if_y == false && if_z == true ) {
                cpml_z.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //xy
            if (if_x == true  && if_y == true  && if_z == false) {
                cpml_xy.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //xz
            if (if_x == true  && if_y == false && if_z == true ) {
                cpml_xz.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //yz
            if (if_x == false && if_y == true  && if_z == true ) {
                cpml_yz.push_back(j_ele+offcet);
                count_cpml_element++;
            }
            //xyz
            if (if_x == true  && if_y == true  && if_z == true ) {
                cpml_xyz.push_back(j_ele+offcet);
                count_cpml_element++;
            }
          
            // check if this element is on the surface of bounding box
            std::vector<int> which_node = check_bb(
                                coordinates[conns[i_vol][j_ele*nnpe]*  dim],   coordinates[conns[i_vol][j_ele*nnpe+1]*dim],
                                coordinates[conns[i_vol][j_ele*nnpe+2]*dim],   coordinates[conns[i_vol][j_ele*nnpe+3]*dim],
                                coordinates[conns[i_vol][j_ele*nnpe+4]*dim],   coordinates[conns[i_vol][j_ele*nnpe+5]*dim],
                                coordinates[conns[i_vol][j_ele*nnpe+6]*dim],   coordinates[conns[i_vol][j_ele*nnpe+7]*dim],

                                coordinates[conns[i_vol][j_ele*nnpe]*  dim+1], coordinates[conns[i_vol][j_ele*nnpe+1]*dim+1], 
                                coordinates[conns[i_vol][j_ele*nnpe+2]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+3]*dim+1], 
                                coordinates[conns[i_vol][j_ele*nnpe+4]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+5]*dim+1], 
                                coordinates[conns[i_vol][j_ele*nnpe+6]*dim+1], coordinates[conns[i_vol][j_ele*nnpe+7]*dim+1], 

                                coordinates[conns[i_vol][j_ele*nnpe]*  dim+2], coordinates[conns[i_vol][j_ele*nnpe+1]*dim+2], 
                                coordinates[conns[i_vol][j_ele*nnpe+2]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+3]*dim+2], 
                                coordinates[conns[i_vol][j_ele*nnpe+4]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+5]*dim+2], 
                                coordinates[conns[i_vol][j_ele*nnpe+6]*dim+2], coordinates[conns[i_vol][j_ele*nnpe+7]*dim+2], 

                                x_bound_min, x_bound_max, y_bound_min, y_bound_max, z_bound_min, z_bound_max );
        
            //store a flag(0: xmin, ~ 5:zmax) and surf id.
            // node combination of each surface in exodusII format
            // surf 0: 1,2,6,5
            // surf 1: 2,3,7,6
            // surf 2: 3,4,8,7
            // surf 3: 4,1,5,8
            // surf 4: 4,3,2,1
            // surf 5: 5,6,7,8
            if (which_node.size() != 0){
               for (int i_flag = 1; i_flag < which_node.size(); i_flag+=2) {
                   std::vector<int> temp;
                   temp.clear(); temp.shrink_to_fit();
                   temp.push_back(offcet+j_ele);

                    if (which_node[i_flag] == 0){
                        temp.push_back(conns[i_vol][j_ele*nnpe]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+1]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+5]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+4]);
                    }
                    else if (which_node[i_flag] == 1){
                        temp.push_back(conns[i_vol][j_ele*nnpe+1]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+2]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+6]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+5]);
                    }
                    else if (which_node[i_flag] == 2) {
                        temp.push_back(conns[i_vol][j_ele*nnpe+2]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+3]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+7]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+6]);
                    }
                    else if (which_node[i_flag] == 3) {
                        temp.push_back(conns[i_vol][j_ele*nnpe+3]);
                        temp.push_back(conns[i_vol][j_ele*nnpe]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+4]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+7]);
                    }
                    else if (which_node[i_flag] == 4) {
                        temp.push_back(conns[i_vol][j_ele*nnpe+3]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+2]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+1]);
                        temp.push_back(conns[i_vol][j_ele*nnpe]);
                    }
                    else if (which_node[i_flag] == 5) {
                        temp.push_back(conns[i_vol][j_ele*nnpe+4]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+5]);                
                        temp.push_back(conns[i_vol][j_ele*nnpe+6]);
                        temp.push_back(conns[i_vol][j_ele*nnpe+7]);
                    }

                    if (which_node[i_flag-1] == 0){
                        for(int j = 0; j < temp.size(); j++)
                            bb_xmi.push_back(temp[j]);
                    }
                    else if (which_node[i_flag-1] == 1){
                         for(int j = 0; j < temp.size(); j++)
                            bb_xma.push_back(temp[j]);
                    }
                    else if (which_node[i_flag-1] == 2) {
                        for(int j = 0; j < temp.size(); j++)
                            bb_ymi.push_back(temp[j]);
                    }
                    else if (which_node[i_flag-1] == 3) {
                        for(int j = 0; j < temp.size(); j++)
                            bb_yma.push_back(temp[j]);
                    }
                    else if (which_node[i_flag-1] == 4) {
                        for(int j = 0; j < temp.size(); j++)
                            bb_zmi.push_back(temp[j]);
                    }
                    else if (which_node[i_flag-1] == 5) {
                        for(int j = 0; j < temp.size(); j++)
                            bb_zma.push_back(temp[j]);
                    }
                }
            }
        }
    } 


/*
    std::cout << "#############################" << '\n';
    std::cout << "## test out from here      ##" << '\n';
    std::cout << "#############################" << '\n';
*/

    /*
    std::cout << "?? test num node per eles" << '\n';
    for (int i = 0; i < num_node_per_elems.size(); ++i)
        std::cout << num_node_per_elems[i] << '\n';
    */
    
    /*
    std::cout << "?? test nodal coords ----" << '\n';
    for(int i = 0; i != coordinates.size(); ++i){
        if (i%3==0) std::cout << "\n" << i/3 << ": ";
        std::cout << coordinates[i] << ", ";
    }
    std::cout << '\n';


    std::cout << "?? test conns -----------" << '\n';   
    int count_bl = 0;
    for(auto row:conns){
        int count = 0;
        std::cout << "? " << '\n';
        for(auto col:row){
            if (count % num_node_per_elems[count_bl] == 0) std::cout << '\n' << count/num_node_per_elems[count_bl] << ": "; 
            std::cout << col << "  ";
            count++;
        }
        std::cout << '\n';
        count_bl++;
    }
    */
    /*
    std::cout << "?? test conns 2 -------" << '\n';
    int ec = 0;
    for (int i = 0; i < num_volume_block; i++){
        for (int j = 0; j < conns[i].size(); j++){
            if(j%num_elems[i] == 0) {
                std::cout << '\n' << ec+1 << ": ";
                ec++;
            }
            std:: cout << conns[i][j] << " ";
        }
    }
    std::cout << '\n';
    */
    /*
    // test attrs 
    std::cout << "? test attributes" << '\n';
    for (int i = 0; i < num_attrs.size(); i++)
        for (int j = 0; j < attrs[i].size() ; j++){
            if (j % num_attrs[i] == 0)
                std::cout << "eleblock: " << i;
            std::cout << "  " <<attrs[i][j] << "  ";
            if (j%num_attrs[i] == num_attrs[i]-1)
                std::cout << '\n';
    }
    */


    std::cout << "####################################" << '\n';
    std::cout << "# read proc end. writtnig out...   #" << '\n';
    std::cout << "####################################" << '\n';

    std::string out_path(argv[2]); // path to out MESH foler
    std::cout << "output folder path: " << out_path << '\n';   

    // definitions of output file names
    std::string mesh_file       = out_path + "mesh_file";
    std::string node_coord      = out_path + "nodes_coords_file";
    std::string mat_file        = out_path + "materials_file";
    std::string num_file        = out_path + "nummaterial_velocity_file";
    std::string top_file        = out_path + "free_or_absorbing_surface_file_zmax";
    std::string abs_xmin_file   = out_path + "absorbing_cpml_file_xmin";
    std::string abs_xmax_file   = out_path + "absorbing_cpml_file_xmax";
    std::string abs_ymin_file   = out_path + "absorbing_cpml_file_ymin";
    std::string abs_yman_file   = out_path + "absorbing_cpml_file_ymax";
    std::string abs_bottom_file = out_path + "absorbing_cpml_file_bottom";
    std::string cpml_file       = out_path + "absorbing_cpml_file";

    std::ofstream write_mesh;
    std::ofstream write_nodes;
    std::ofstream write_mat;
    std::ofstream write_num;
    std::ofstream write_top;
    std::ofstream write_xmi;
    std::ofstream write_xma;
    std::ofstream write_ymi;
    std::ofstream write_yma;
    std::ofstream write_bot;
    std::ofstream write_cpml;

    write_mesh.open(mesh_file     ,std::ios::out);
    write_nodes.open(node_coord   ,std::ios::out);
    write_mat.open(mat_file       ,std::ios::out);
    write_num.open(num_file       ,std::ios::out);
    write_num.precision(10);
    write_top.open(top_file       ,std::ios::out); 
    write_xmi.open(abs_xmin_file  ,std::ios::out);
    write_xma.open(abs_xmax_file  ,std::ios::out);
    write_ymi.open(abs_ymin_file  ,std::ios::out);
    write_yma.open(abs_yman_file  ,std::ios::out);
    write_bot.open(abs_bottom_file,std::ios::out);
    write_cpml.open(cpml_file     ,std::ios::out);


    std::cout << "writing mesh file..." << '\n';
    int elem_id = 1;
    int one_line[num_node_per_elems[0]];
    int reordered_line[num_node_per_elems[0]];
    int i_conn = 0;

    write_mesh << total_num_elems << '\n';
    for (int i = 0; i < num_volume_block; ++i){
        if(num_node_per_elems[i] == 8){
            for (int j = 0; j < conns[i].size(); ++j){
                
                i_conn = j % num_node_per_elems[i];
                
                if (i_conn  == 0 ){
                    write_mesh << elem_id << " ";
                    elem_id++;
                }
                
                write_mesh << conns[i][j] + 1 << " ";
                
                if ( i_conn == num_node_per_elems[i]-1 )
                    write_mesh << '\n';
            }
        } else { // 27 nodes
            for (int j = 0; j < conns[i].size(); ++j){
                i_conn = j % num_node_per_elems[i];
                if ( i_conn == 0 ){
                        write_mesh << elem_id << " ";
                        elem_id++;
                }
               
                one_line[i_conn] = conns[i][j];

                if ( i_conn == num_node_per_elems[i]-1 ){
                    for(int k = 0; k < 20; ++k)
                        reordered_line[k] = one_line[k];

                    reordered_line[20] = one_line[21];
                    reordered_line[21] = one_line[25];
                    reordered_line[22] = one_line[24];
                    reordered_line[23] = one_line[26];
                    reordered_line[24] = one_line[23];
                    reordered_line[25] = one_line[22];
                    reordered_line[26] = one_line[20];


                    for (int k = 0; k < num_node_per_elems[i]; ++k)
                        write_mesh << reordered_line[k]+1 << " ";
                    write_mesh << '\n';
                }
            }  

        }
    }


    std::cout << "writing node coord..." << '\n';
    write_nodes << total_num_nodes << '\n';
    write_nodes.precision(8);
    for (int AA = 0; AA < total_num_nodes; AA++){
        write_nodes << AA+1 << " ";
        //cout << AA+1 << " ";
        for (int BB = 0; BB < dim; BB++){
            //write_nodes << node_coords[invert_node_map[AA]][BB] << " ";
            write_nodes << coordinates[AA*dim + BB] << " ";
        }
        write_nodes << '\n';
    }


    ///////////////////////////////////////////////
    //
    // we here put the volume id = material id as attributes are not configurable from gmsh
    //
    ///////////////////////////////////////////////
    std::cout << "writing material file..." << '\n';
    elem_id = 1;
    for (int i = 0; i < num_volume_block; ++i){
        for (int j = 0; j < num_elems[i]; ++j){
            write_mat << elem_id << "   " << i+1 << '\n';
            //std::cout << "i,j: " << (int)i << ", " << j << ", elem_id " << elem_id << '\n';
            elem_id++;
        }
    }

    ////////////////////////////////////////////////
    //
    // nummaterial velocity file need to be modified manually
    //
    //////////////////////////////////////////////////
    std::cout << "writing nummaterial velocity file..." << '\n';
//    for (int i_block = 0; i_block < num_volume_block; i_block++){
//        write_num << i_block+1 << "  " << attrs[i_block][0] 
//                               << "  " << attrs[i_block][3] 
//                               << "  " << attrs[i_block][1]
//                               << "  " << attrs[i_block][2]
//                               << "  " << attrs[i_block][4]
//                               << "  " << attrs[i_block][5]
//                               << "  " << attrs[i_block][6];
//        write_num << '\n';
//    }


    write_num << '\n' << '\n' << '\n' << '\n';
    write_num << "! note: format of nummaterial_velocity_file must be                                                             " << '\n';                                                           
    write_num << "                                                                                                                " << '\n';
    write_num << "! #(1)material_domain_id #(2)material_id  #(3)rho  #(4)vp   #(5)vs   #(6)Q_kappa   #(7)Q_mu  #(8)anisotropy_flag" << '\n';
    write_num << "!                                                                                                               " << '\n';
    write_num << "! where                                                                                                         " << '\n';
    write_num << "!     material_domain_id : 1=acoustic / 2=elastic                                                               " << '\n';
    write_num << "!     material_id        : POSITIVE integer identifier corresponding to the identifier of material block        " << '\n';
    write_num << "!     rho                : density                                                                              " << '\n';
    write_num << "!     vp                 : P-velocity                                                                           " << '\n';
    write_num << "!     vs                 : S-velocity                                                                           " << '\n';
    write_num << "!     Q_kappa            : 9999 = no Q_kappa attenuation                                                        " << '\n';
    write_num << "!     Q_mu               : 9999 = no Q_mu attenuation                                                           " << '\n';
    write_num << "!     anisotropy_flag    : 0=no anisotropy/ 1,2,.. check with implementation in aniso_model.f90                 " << '\n';
    write_num << "!                                                                                                               " << '\n';
    write_num << "!example:                                                                                                       " << '\n';
    write_num << "!2   1 2300 2800 1500 9999.0 9999.0 0                                                                           " << '\n';
    write_num << "                                                                                                                " << '\n';
    write_num << "!or                                                                                                             " << '\n';
    write_num << "                                                                                                                " << '\n';
    write_num << "! #(1)material_domain_id #(2)material_id  tomography elastic  #(3)tomography_filename #(4)positive_unique_number" << '\n';
    write_num << "!                                                                                                               " << '\n';
    write_num << "! where                                                                                                         " << '\n';
    write_num << "!     material_domain_id : 1=acoustic / 2=elastic                                                               " << '\n';
    write_num << "!     material_id        : NEGATIVE integer identifier corresponding to the identifier of material block        " << '\n';
    write_num << "!     tomography_filename: filename of the tomography file                                                      " << '\n';
    write_num << "!     positive_unique_number: a positive unique identifier                                                      " << '\n';
    write_num << "!                                                                                                               " << '\n';
    write_num << "!example:                                                                                                       " << '\n';
    write_num << "!2  -1 tomography elastic tomo.xyz 1                                                                            " << '\n';
 


    std::cout << "writing free or absorbing file..." << '\n';

    //top
    write_top << bb_zma.size()/5 << '\n';
    for (int j_ele = 0; j_ele < bb_zma.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_top << bb_zma[j_ele]+1 << " " << bb_zma[j_ele+1]+1 << " " << bb_zma[j_ele+2]+1 << " " << bb_zma[j_ele+3]+1 << " " << bb_zma[j_ele+4]+1 << '\n';
        
    //xmin
    write_xmi << bb_xmi.size()/5 << '\n';
    for (int j_ele = 0; j_ele < bb_xmi.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_xmi << bb_xmi[j_ele]+1 << " " << bb_xmi[j_ele+1]+1 << " " << bb_xmi[j_ele+2]+1 << " " << bb_xmi[j_ele+3]+1 << " " << bb_xmi[j_ele+4]+1 << '\n';
 
    //xmax
    write_xma << bb_xma.size()/5 << '\n';
    for (int j_ele = 0; j_ele < bb_xma.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_xma << bb_xma[j_ele]+1 << " " << bb_xma[j_ele+1]+1 << " " << bb_xma[j_ele+2]+1 << " " << bb_xma[j_ele+3]+1 << " " << bb_xma[j_ele+4]+1 << '\n';
    
    //ymin
    write_ymi << bb_ymi.size()/5 << '\n';
    for (int j_ele = 0; j_ele < bb_ymi.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_ymi << bb_ymi[j_ele]+1 << " " << bb_ymi[j_ele+1]+1 << " " << bb_ymi[j_ele+2]+1 << " " << bb_ymi[j_ele+3]+1 << " " << bb_ymi[j_ele+4]+1 << '\n';
 
    //ymax
    write_yma << bb_yma.size()/5<< '\n';
    for (int j_ele = 0; j_ele < bb_yma.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_yma << bb_yma[j_ele]+1 << " " << bb_yma[j_ele+1]+1 << " " << bb_yma[j_ele+2]+1 << " " << bb_yma[j_ele+3]+1 << " " << bb_yma[j_ele+4]+1 << '\n';
 
    //bottom
    write_bot << bb_zmi.size()/5 << '\n';
    for (int j_ele = 0; j_ele < bb_zmi.size(); j_ele+=5) // 5 = ele_id + 4 corner nodes of a face
        write_bot << bb_zmi[j_ele]+1 << " " << bb_zmi[j_ele+1]+1 << " " << bb_zmi[j_ele+2]+1 << " " << bb_zmi[j_ele+3]+1 << " " << bb_zmi[j_ele+4]+1 << '\n';
    


    std::cout << "writing cpml file..." << '\n';
    write_cpml << count_cpml_element << '\n';

    //write x
    for(auto itr = cpml_x.begin(); itr != cpml_x.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 1 << '\n';
    }
    //write y
    for(auto itr = cpml_y.begin(); itr != cpml_y.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 2 << '\n';
    }
    //write z
    for(auto itr = cpml_z.begin(); itr != cpml_z.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 3 << '\n';
    }
    //write xy
    for(auto itr = cpml_xy.begin(); itr != cpml_xy.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 4 << '\n';
    }
    //write xz
    for(auto itr = cpml_xz.begin(); itr != cpml_xz.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 5 << '\n';
    }
    //write yz
    for(auto itr = cpml_yz.begin(); itr != cpml_yz.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 6 << '\n';
    }
    //write xyz
    for(auto itr = cpml_xyz.begin(); itr != cpml_xyz.end(); ++itr ){
        write_cpml << *itr+1 << "   " << 7 << '\n';
    }
}
