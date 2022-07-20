import { Injectable } from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { Model } from 'mongoose';
import { CreateUserInput, User, UserDocument } from './user.schema';

@Injectable()
export class UserService {
    constructor(@InjectModel(User.name) private userModel: Model<UserDocument>){}

    async findMany() {
        return this.userModel.find().lean();
    }

    async findById(id) {
        return this.userModel.findById(id).lean();
    }

    async createUser(user: CreateUserInput) {
        return this.userModel.create(user);
    }
}
